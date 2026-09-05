#include <Editor/FindOnsets.h>

#include <Core/Utils.h>
#include <Core/AlignedMemory.h>

#include <System/Thread.h>

#include <aubio/aubio.h>
#include <math.h>
#include <mutex>
#include <vector>
#include <cstring>

namespace Vortex {

// ================================================================================================
// Main function.

void FindOnsets(const float* samples, int samplerate, int numFrames,
                int numThreads, std::vector<Onset>& out) {
    static const int windowlen = 256;
    static const int bufsize = windowlen * 4;
    static const char* method = "complex";

    auto onset = new_aubio_onset(method, bufsize, windowlen, samplerate);
    if (!onset) return;

    // Do not clear aubio's delay. Positions come out of
    // aubio_onset_get_last(), which returns (last_onset - delay); that delay
    // compensates for the group delay of the peak picker, which only confirms a
    // peak several hops after the audio that caused it. For the "complex"
    // method it defaults to 4.6 * hop = 1177 samples.
    //
    // It looks at first glance like a streaming-only allowance that an offline
    // caller such as this one should drop, and dropping it does shift every
    // reported onset about 25 ms later. It is not: measured against a metronome
    // whose clicks sit on exact 0.25 s boundaries, aubio's reported positions
    // average 5.2 ms *before* the true attack with the delay left alone, and
    // 19.3 ms *after* it once the delay is cleared. Leaving it alone is right.
    //
    // Beware of validating this against hand-synced charts. Charts synced by
    // feel carry their own systematic bias, easily tens of milliseconds, and
    // measuring against them can make the detector look wrong in whichever
    // direction the charts happen to lean. Use material with a known-exact
    // grid.

    fvec_t *samplevec = new_fvec(windowlen), *beatvec = new_fvec(2);
    for (int i = 0; i <= numFrames - windowlen; i += windowlen) {
        std::memcpy(samplevec->data, samples + i, sizeof(float) * windowlen);
        aubio_onset_do(onset, samplevec, beatvec);
        if (beatvec->data[0] > 0) {
            int pos = aubio_onset_get_last(onset);
            Onset new_onset = {pos, 1.0f};
            if (pos >= 0) out.emplace_back(new_onset);
        }
    }
    del_fvec(samplevec);
    del_fvec(beatvec);
    del_aubio_onset(onset);
}

};  // namespace Vortex