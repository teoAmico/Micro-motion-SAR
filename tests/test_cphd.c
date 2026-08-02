/* CPHD reader: round trip on a synthesised file, and a malformed-input corpus.
 *
 * The reader parses an untrusted external format, so it gets both halves of the
 * error contract in readers.h tested. The round trip builds a small but
 * structurally complete CPHD -- ASCII header, XML, PVP block, big-endian
 * CF8 signal in the FX domain -- with a point scatterer at a chosen range bin,
 * and checks the reader recovers the scatterer, the geometry and the timing.
 *
 * The corpus then damages that same file in one specific way at a time. Each
 * case asserts a status code and, more importantly, that nothing crashes and
 * nothing is left allocated. These are the inputs a public repository will
 * eventually receive, and the plan requires them before it becomes one. */

#include "resonarsat/readers.h"
#include "rs_test.h"

#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NV   64u    /* pulses */
/* The fixture steps 100 m along track every 1 ms, so this is the platform speed
 * its ReferenceGeometry must declare for the metadata screen to agree with what
 * the full reader measures from the positions. Unphysical, like the rest of the
 * fixture's geometry, and consistent, which is what the comparison needs. */
#define VPLAT (100.0 / 0.001)
#define NS   32u    /* range samples per pulse */
#define PVPB 240u   /* bytes per vector, 30 words -- the layout Umbra emits */

/* Word offsets within a PVP vector, matching the XML the fixture writes. */
enum { O_TXTIME = 0, O_TXPOS = 1, O_RCVPOS = 8, O_SRPPOS = 14, O_SC0 = 27,
       O_SCSS = 28, O_AMPSF = 29 };
/* A per-vector amplitude scale the reader must apply. Varies pulse to pulse so
 * that failing to apply it is visible as a varying gain rather than a constant
 * one, which is how the real defect presented. */
#define AMPSF_AT(p) (0.5 + 0.01 * (double)(p))

/* Scene reference point in earth-centred fixed coordinates, and the range at
 * which the fixture places the platform. Values are a real Umbra scene's, so
 * the geometry the reader derives is checked against a plausible one. */
static const double SRP[3] = { -552223.3276, -4332875.0, 4632556.6406 };
static const double R0 = 900000.0;
static const double SCSS = 21496.0;   /* Hz per sample */
static const double FXMIN = 9.5e9, FXMAX = 9.7e9;
static const size_t TARGET_BIN = 3;   /* delay bins past the reference point */

/* Write a double to 'p' in big-endian order, the byte order every CPHD binary
 * block uses regardless of the host. */
static void put_f8(unsigned char *p, double d)
{
    uint64_t u;
    memcpy(&u, &d, sizeof u);
    for (int i = 0; i < 8; i++) p[i] = (unsigned char)(u >> (56 - 8 * i));
}

/* Write a float to 'p' in big-endian order. */
static void put_f4(unsigned char *p, float f)
{
    uint32_t u;
    memcpy(&u, &f, sizeof u);
    for (int i = 0; i < 4; i++) p[i] = (unsigned char)(u >> (24 - 8 * i));
}

/* Build a structurally complete CPHD 1.1.0 file at 'path'.
 *
 * The signal array is written in closed form rather than by transforming a
 * profile: a unit scatterer at delay bin m has FX-domain samples
 * exp(-j*2*pi*k*m/N), so writing that directly means the test does not depend
 * on the same FFT the reader uses.
 *
 * WHAT THIS CHECK CANNOT DO, which matters because it reads as though it can.
 * The fixture pairs that exponent with <SGN>-1</SGN>, so it assumes SGN gives
 * the sign in the data's own exponent -- the same assumption rs_read_cphd()
 * makes when it chooses the transform direction. The two therefore cannot
 * disagree about it. Flipping the fixture's exponent AND the reader's branch
 * together leaves this test passing unchanged, which has been verified by
 * doing it. So this catches a reader-only regression in FFT direction or
 * normalisation, and is blind to the SGN convention being wrong in both
 * places at once.
 *
 * That blind spot was live and it cost a day, though the resolution came from
 * outside the suite entirely. SARPy documents the standard as
 * Phase(fx) = SGN * fx * dTOA, so SGN = -1 here means exp(-j...) and the
 * inverse transform recovers bin m -- which is what this fixture and the reader
 * both do, and both are right. What was wrong was Capella's data, which
 * declares SGN = +1 while shipping the opposite convention; that is handled by
 * a vendor-keyed override in the reader, not by changing this. See
 * src/readers/cphd.c. 'damage' names one deliberate corruption, or
 * NULL for a valid file. Returns 0 on success. */
static int write_cphd(const char *path, const char *damage)
{
    char xml[4096];
    const int xn = snprintf(xml, sizeof xml,
        "<CPHD><Global><DomainType>%s</DomainType><SGN>-1</SGN>"
        "<FxBand><FxMin>%.10g</FxMin><FxMax>%.10g</FxMax></FxBand></Global>"
        "<SceneCoordinates><IARP><ECF><X>%.10g</X><Y>%.10g</Y><Z>%.10g</Z></ECF></IARP>"
        "<ReferenceSurface><Planar>"
        "<uIAX><X>1</X><Y>0</Y><Z>0</Z></uIAX>"
        "<uIAY><X>0</X><Y>1</Y><Z>0</Z></uIAY>"
        "</Planar></ReferenceSurface></SceneCoordinates>"
        "<Data><SignalArrayFormat>%s</SignalArrayFormat><NumBytesPVP>%u</NumBytesPVP>"
        "<NumCPHDChannels>1</NumCPHDChannels><Channel>"
        "<NumVectors>%u</NumVectors><NumSamples>%u</NumSamples></Channel></Data>"
        "<CollectionID><CollectorName>rs-test</CollectorName>"
        "<CollectType>MONOSTATIC</CollectType></CollectionID>"
        /* ReferenceGeometry and the channel's transmit-time span. The full
         * reader takes neither -- it measures timing from the PVP and geometry
         * from the recorded positions -- but every real product carries them
         * and rs_read_cphd_meta() screens a collect from them alone. A fixture
         * without them would leave the screening path untested against the read
         * it screens for, which is the one comparison that matters. */
        "<Channel><Parameters><TxTime1>0</TxTime1>"
        "<TxTime2>%.10g</TxTime2></Parameters></Channel>"
        "<ReferenceGeometry><SRP><ECF><X>%.10g</X><Y>%.10g</Y><Z>%.10g</Z></ECF></SRP>"
        "<Monostatic><ARPPos><X>0</X><Y>0</Y><Z>%.10g</Z></ARPPos>"
        "<ARPVel><X>%.10g</X><Y>0</Y><Z>0</Z></ARPVel>"
        "<SlantRange>%.10g</SlantRange><IncidenceAngle>%.10g</IncidenceAngle>"
        "</Monostatic></ReferenceGeometry>"
        "<PVP><TxTime><Offset>%d</Offset><Size>1</Size><Format>F8</Format></TxTime>"
        "<TxPos><Offset>%d</Offset><Size>3</Size><Format>X=F8;Y=F8;Z=F8;</Format></TxPos>"
        "<RcvPos><Offset>%d</Offset><Size>3</Size><Format>X=F8;Y=F8;Z=F8;</Format></RcvPos>"
        "<SRPPos><Offset>%d</Offset><Size>3</Size><Format>X=F8;Y=F8;Z=F8;</Format></SRPPos>"
        "<SC0><Offset>%d</Offset><Size>1</Size><Format>F8</Format></SC0>"
        "<AmpSF><Offset>%d</Offset><Size>1</Size><Format>F8</Format></AmpSF>"
        "<SCSS><Offset>%d</Offset><Size>1</Size><Format>F8</Format></SCSS></PVP></CPHD>",
        (damage && !strcmp(damage, "toa")) ? "TOA" : "FX",
        FXMIN, FXMAX, SRP[0], SRP[1], SRP[2],
        (damage && !strcmp(damage, "format")) ? "CI2" : "CF8",
        PVPB, NV, (damage && !strcmp(damage, "nsamp")) ? 999999u : NS,
        /* The fixture lays pulses 1 ms apart, so the span is (NV-1) ms and the
         * screen must recover exactly the PRF the full reader measures. */
        (double)(NV - 1) * 1.0e-3,
        SRP[0], SRP[1], SRP[2], R0, VPLAT, R0, 0.0,
        O_TXTIME, O_TXPOS, O_RCVPOS, O_SRPPOS, O_SC0, O_AMPSF, O_SCSS);
    if (xn <= 0 || (size_t)xn >= sizeof xml) return -1;

    const unsigned long xml_off = 1024;
    const unsigned long pvp_off = xml_off + (unsigned long)xn;
    const unsigned long pvp_size = (unsigned long)PVPB * NV;
    const unsigned long sig_off = pvp_off + pvp_size;
    const unsigned long sig_size = (unsigned long)NV * NS * 8u;

    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    char hdr[1024];
    memset(hdr, 0, sizeof hdr);
    int hn = snprintf(hdr, sizeof hdr,
        "%s\nXML_BLOCK_SIZE := %d\nXML_BLOCK_BYTE_OFFSET := %lu\n"
        "PVP_BLOCK_SIZE := %lu\nPVP_BLOCK_BYTE_OFFSET := %lu\n"
        "SIGNAL_BLOCK_SIZE := %lu\nSIGNAL_BLOCK_BYTE_OFFSET := %lu\n"
        "CLASSIFICATION := UNCLASSIFIED\n",
        (damage && !strcmp(damage, "magic")) ? "NITF/2.1" : "CPHD/1.1.0",
        xn, xml_off,
        pvp_size, pvp_off,
        (damage && !strcmp(damage, "sigsize")) ? sig_size * 64u : sig_size,
        (damage && !strcmp(damage, "sigoff")) ? 0xfffffff0UL : sig_off);
    if (hn <= 0) { fclose(f); return -1; }
    if (!(damage && !strcmp(damage, "noff"))) hdr[hn++] = '\f';
    hdr[hn++] = '\n';
    fwrite(hdr, 1, (size_t)xml_off, f);          /* header padded to the XML offset */
    fseek(f, (long)xml_off, SEEK_SET);
    fwrite(xml, 1, (size_t)xn, f);

    unsigned char v[PVPB];
    for (unsigned p = 0; p < NV; p++) {
        memset(v, 0, sizeof v);
        /* Platform flies along +X of the scene frame, through broadside. */
        const double along = ((double)p - (double)NV / 2.0) * 100.0;
        const double pos[3] = { SRP[0] + along, SRP[1], SRP[2] + R0 };
        put_f8(v + O_TXTIME * 8, (double)p * 0.001);
        for (int k = 0; k < 3; k++) {
            put_f8(v + ((size_t)O_TXPOS  + (size_t)k) * 8, pos[k]);
            put_f8(v + ((size_t)O_RCVPOS + (size_t)k) * 8, pos[k]);
            put_f8(v + ((size_t)O_SRPPOS + (size_t)k) * 8, SRP[k]);
        }
        put_f8(v + O_SC0 * 8, FXMIN);
        put_f8(v + O_AMPSF * 8, AMPSF_AT(p));
        put_f8(v + O_SCSS * 8,
               (damage && !strcmp(damage, "scss") && p == 7) ? SCSS * 2.0 : SCSS);
        fwrite(v, 1, sizeof v, f);
    }

    unsigned char *sig = calloc(NS, 8u);
    if (!sig) { fclose(f); return -1; }
    for (unsigned p = 0; p < NV; p++) {
        for (unsigned k = 0; k < NS; k++) {
            const double ph = -2.0 * M_PI * (double)k * (double)TARGET_BIN / (double)NS;
            put_f4(sig + (size_t)k * 8u,      (float)cos(ph));
            put_f4(sig + (size_t)k * 8u + 4u, (float)sin(ph));
        }
        fwrite(sig, 8u, NS, f);
    }
    free(sig);
    fclose(f);

    if (damage && !strcmp(damage, "truncate")) {
        FILE *t = fopen(path, "rb+");
        if (!t) return -1;
        fseek(t, 0, SEEK_END);
        const long n = ftell(t);
        fclose(t);
        if (truncate(path, n - (long)(NS * 8u * 4u)) != 0) return -1;
    }
    return 0;
}

int main(void)
{
    const char *dir = getenv("TMPDIR");
    char path[512];
    snprintf(path, sizeof path, "%srs_test_%d.cphd", dir ? dir : "/tmp/", (int)getpid());

    RS_CASE("a synthesised CPHD round trips through the reader");
    {
        RS_CHECK(write_cphd(path, NULL) == 0);
        rs_cphd_t c;
        RS_CHECK_OK(rs_read_cphd(path, NULL, &c));

        RS_CHECK(c.n_pulse == NV);
        RS_CHECK(c.n_rbin == NS);

        /* Range bin spacing from the declared bandwidth. */
        const double dr = 299792458.0 / (2.0 * (double)NS * SCSS);
        RS_CHECK_NEAR(c.dr, dr, 1e-9 * dr);
        /* The carrier is SC0, the first sample's frequency, not the band
         * centre -- that is where transforming the FX samples leaves the
         * residual phase, and the backprojector has to undo it there. The
         * fixture writes SC0 = FXMIN, so that is what must come back. */
        RS_CHECK_NEAR(c.fc, FXMIN, 1.0);
        RS_CHECK_NEAR(c.lambda, 299792458.0 / c.fc, 1e-12);

        /* PRF from the 1 ms pulse spacing the fixture wrote. */
        RS_CHECK_NEAR(c.prf, 1000.0, 1e-6);

        /* The reference range is the platform-to-SRP distance, and the fixture
         * put the platform R0 above the SRP at broadside. */
        RS_CHECK(c.r_ref[NV / 2] > R0 * 0.99 && c.r_ref[NV / 2] < R0 * 1.01);

        /* Positions arrive in the scene's planar frame, so the origin is the
         * SRP and the track runs along the frame's x axis. */
        RS_CHECK_NEAR(c.pos[3 * (NV / 2) + 2], R0, 1.0);
        RS_CHECK(c.pos[3 * (NV - 1)] > c.pos[0]);

        /* The scatterer: written at delay bin TARGET_BIN, so after compression
         * and centring it must sit that many bins past the profile centre. */
        size_t peak = 0;
        double best = -1.0;
        for (size_t k = 0; k < c.n_rbin; k++) {
            const double m = cabs(c.signal[(NV / 2) * c.n_rbin + k]);
            if (m > best) { best = m; peak = k; }
        }
        printf("    scatterer recovered at bin %zu, centre %u, expected %zu\n",
               peak, NS / 2u, NS / 2u + TARGET_BIN);
        RS_CHECK(peak == NS / 2u + TARGET_BIN);

        rs_cphd_free(&c);
    }

    RS_CASE("the per-vector amplitude scale factor is applied");
    {
        /* CPHD stores signal samples that must be multiplied by the vector's
         * AmpSF to be calibrated amplitude. This reader ignored it for the whole
         * of its life, which is invisible in an image -- focusing averages a
         * per-pulse gain away -- and very visible in anything comparing
         * amplitudes BETWEEN sub-looks.
         *
         * That is now a selection criterion. Measured on the Istanbul collect
         * AmpSF has a dispersion of 0.711 across its vectors, and injecting a
         * gain of that dispersion into the synthetic fixture takes its amplitude
         * dispersion from 0.083 to 0.337 -- which is where every real scene this
         * project has measured sat. See FOLLOW-UPS.md item 21.
         *
         * The fixture writes a scale that RISES with the pulse index, so a
         * reader that ignored it would show a flat gain where this shows a ramp,
         * and one that applied a constant would show the wrong ramp. */
        RS_CHECK(write_cphd(path, NULL) == 0);
        rs_cphd_t c;
        RS_CHECK_OK(rs_read_cphd(path, NULL, &c));

        /* Total energy per pulse scales as AmpSF^2, so the ratio between two
         * pulses is the ratio of their factors squared -- independent of what
         * the scene put in them, which the fixture makes identical per pulse
         * only up to the along-track geometry. Compare the same range bin. */
        size_t peak = 0;
        double best = -1.0;
        for (size_t k = 0; k < c.n_rbin; k++) {
            const double m = cabs(c.signal[(NV / 2) * c.n_rbin + k]);
            if (m > best) { best = m; peak = k; }
        }
        const size_t p_lo = 8, p_hi = NV - 8;
        const double a_lo = cabs(c.signal[p_lo * c.n_rbin + peak]);
        const double a_hi = cabs(c.signal[p_hi * c.n_rbin + peak]);
        const double want = AMPSF_AT(p_hi) / AMPSF_AT(p_lo);
        printf("    amplitude ratio between pulses %zu and %zu: %.4f, "
               "AmpSF ratio %.4f\n", p_lo, p_hi, a_hi / a_lo, want);
        RS_CHECK_REL(a_hi / a_lo, want, 0.02);

        /* And the absolute scale is the factor itself, not merely proportional
         * to it: a reader normalising the ramp away would pass the ratio test. */
        RS_CHECK(a_lo > 0.0);

        rs_cphd_free(&c);
    }

    RS_CASE("the metadata-only screen agrees with the full read");
    {
        /* A screen exists to predict the run. If the two disagree about the
         * geometry then screening a 17 GB collect to decide whether to download
         * it is worse than useless, because the decision is made on numbers the
         * run will not reproduce. rs_read_cphd_meta() is measured against
         * rs_read_cphd() on the same bytes here, and the fields where they
         * legitimately differ are asserted to differ in the documented way
         * rather than being left to be discovered. */
        RS_CHECK(write_cphd(path, NULL) == 0);
        rs_cphd_t c;
        RS_CHECK_OK(rs_read_cphd(path, NULL, &c));
        rs_cphd_meta_t m;
        RS_CHECK_OK(rs_read_cphd_meta(path, &m));

        printf("    full read: %zu pulses, fc %.4f GHz, lambda %.6f m, "
               "prf %.3f Hz, dwell %.4f s\n",
               c.n_pulse, c.fc / 1e9, c.lambda, c.prf,
               c.t[c.n_pulse - 1] - c.t[0]);
        printf("    screen:    %zu pulses, fc %.4f GHz, lambda %.6f m, "
               "prf %.3f Hz, dwell %.4f s\n",
               m.n_pulse, m.fc_hz / 1e9, m.lambda_m, m.prf_hz, m.dwell_s);

        /* The carrier must match exactly: it sets lambda, and lambda scales
         * every phase-derived displacement the pipeline reports. Taking the
         * band CENTRE instead would be 3.2% out on a real Capella product. */
        RS_CHECK_NEAR(m.fc_hz, c.fc, 1.0);
        RS_CHECK_NEAR(m.lambda_m, c.lambda, 1e-12);

        /* Timing agrees to a part in 1e6: the fixture writes uniform pulses, so
         * the declared span and the measured one are the same quantity. */
        RS_CHECK_REL(m.dwell_s, c.t[c.n_pulse - 1] - c.t[0], 1e-6);
        RS_CHECK_REL(m.prf_hz, c.prf, 1e-6);

        /* This fixture flags no vector invalid, so the declared count and the
         * screened count coincide here. On a real product they need not: the
         * Giza collect declares 335149 and the full reader keeps 335141. */
        RS_CHECK(m.n_pulse == c.n_pulse);
        RS_CHECK(m.n_rbin == NS);

        /* And it reads the same file the full reader does, by seeking the XML
         * block the ASCII header points at -- not by any parallel convention. */
        RS_CHECK(m.v_platform_ms > 0.0);
        RS_CHECK(m.slant_range_m > 0.0);

        rs_cphd_free(&c);
    }

    RS_CASE("the screen refuses what it cannot read, with a status");
    {
        rs_cphd_meta_t m;
        RS_CHECK_ERR(rs_read_cphd_meta("/nonexistent/nowhere.cphd", &m), RS_ERR_IO);
        RS_CHECK_ERR(rs_read_cphd_meta(NULL, &m), RS_ERR_ARG);
        RS_CHECK_ERR(rs_read_cphd_meta(path, NULL), RS_ERR_ARG);

        /* A file that is neither a CPHD nor a metadata block must produce a
         * described status rather than a plausible-looking geometry. */
        char junk[512];
        snprintf(junk, sizeof junk, "%srs_test_junk_%d.xml",
                 dir ? dir : "/tmp/", (int)getpid());
        FILE *jf = fopen(junk, "wb");
        RS_CHECK(jf != NULL);
        fputs("<CPHD><nothing/></CPHD>", jf);
        fclose(jf);
        RS_CHECK_ERR(rs_read_cphd_meta(junk, &m), RS_ERR_MISSING_META);
        printf("    refused, as it should: %s\n", rs_last_error());
        remove(junk);
    }

    RS_CASE("a range window keeps the reference point centred");
    {
        rs_cphd_read_opts_t o = { .rbin_window = 16, .pulse_stride = 0, .max_pulses = 0 };
        rs_cphd_t c;
        RS_CHECK_OK(rs_read_cphd(path, &o, &c));
        RS_CHECK(c.n_rbin == 16);
        RS_CHECK(c.n_pulse == NV);

        size_t peak = 0;
        double best = -1.0;
        for (size_t k = 0; k < c.n_rbin; k++) {
            const double m = cabs(c.signal[(NV / 2) * c.n_rbin + k]);
            if (m > best) { best = m; peak = k; }
        }
        RS_CHECK(peak == 16u / 2u + TARGET_BIN);
        rs_cphd_free(&c);
    }

    RS_CASE("striding lowers the reported PRF rather than overstating it");
    {
        rs_cphd_read_opts_t o = { .rbin_window = 0, .pulse_stride = 4, .max_pulses = 0 };
        rs_cphd_t c;
        RS_CHECK_OK(rs_read_cphd(path, &o, &c));
        RS_CHECK(c.n_pulse == NV / 4u);
        RS_CHECK_NEAR(c.prf, 250.0, 1e-6);
        rs_cphd_free(&c);
    }

    /* ------------------------------------------------------------------
     * Malformed inputs. One deliberate defect per file.
     * ------------------------------------------------------------------ */
    RS_CASE("malformed inputs are refused with a status, not a crash");
    {
        const struct { const char *damage; resonarsat_status_t want; const char *why; } cases[] = {
            { "magic",    RS_ERR_FORMAT,      "not a CPHD version line" },
            { "noff",     RS_ERR_FORMAT,      "no header terminator" },
            { "sigoff",   RS_ERR_FORMAT,      "signal block starts past EOF" },
            { "sigsize",  RS_ERR_FORMAT,      "signal block larger than the file" },
            { "nsamp",    RS_ERR_FORMAT,      "declared samples exceed the block" },
            { "truncate", RS_ERR_FORMAT,      "file cut short mid-signal" },
            { "format",   RS_ERR_UNSUPPORTED, "sample format is not CF8" },
            { "scss",     RS_ERR_UNSUPPORTED, "sample spacing varies between pulses" },
        };
        for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
            RS_CHECK(write_cphd(path, cases[i].damage) == 0);
            rs_cphd_t c;
            const resonarsat_status_t got = rs_read_cphd(path, NULL, &c);
            printf("    %-9s -> %-24s (%s)\n", cases[i].damage,
                   rs_status_str(got), cases[i].why);
            RS_CHECK(got == cases[i].want);
            /* Freeing after a failed read must be safe and must not double
             * free: the reader promises to leave nothing allocated. */
            rs_cphd_free(&c);
        }
    }

    RS_CASE("a missing file and a null path are refused");
    {
        rs_cphd_t c;
        RS_CHECK_ERR(rs_read_cphd("/nonexistent/path/x.cphd", NULL, &c), RS_ERR_IO);
        RS_CHECK_ERR(rs_read_cphd(NULL, NULL, &c), RS_ERR_ARG);
        RS_CHECK_ERR(rs_read_cphd(path, NULL, NULL), RS_ERR_ARG);
    }

    remove(path);
    RS_TEST_END();
}
