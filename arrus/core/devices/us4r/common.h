#ifndef ARRUS_CORE_DEVICES_US4R_COMMON_H
#define ARRUS_CORE_DEVICES_US4R_COMMON_H

#include <vector>
#include <tuple>

#include "arrus/core/api/common/types.h"
#include "arrus/common/asserts.h"
#include "arrus/core/devices/TxRxParameters.h"
#include "arrus/core/devices/us4r/us4oem/Us4OEMImpl.h"

#include "arrus/core/external/eigen/Tensor.h"

namespace arrus::devices {


struct SplitResult {
    std::vector<TxRxParamsSequence> sequences;
    /* a mapping (module, input op index, rx channel) -> output (physical) frame number */
    Eigen::Tensor<FrameChannelMapping::FrameNumber, 3> frames;
    /* a mapping (module, input op index, rx channel) -> output (physical) frame rx channel */
    Eigen::Tensor<int8, 3> channels;
    /** A list of updated constants */
    std::unordered_map<Ordinal, std::vector<arrus::framework::NdArray>> constants;
    std::vector<std::pair<uint16_t, uint16_t>> logicalToPhysicalOp;
};

/**
 * Splits each tx/rx operation into multiple ops so that each rx aperture
 * does not include the same rx channel multiple times.
 *
 * This function is intended to be used for Us4OEM TxRxs only!
 *
 * Note: us4oems have 32 rx channels, however 128 rx channels are addressable;
 * each addressable rx channel 'i' is connected to us4oem channel 'i modulo 32',
 * so for example us4oem channel 0 can handle the output addressable channels
 * 0, 32, 64, 96; only one of these channels can be set in a single Rx aperture.
 *
 * `Seqs` input parameter is a vector of sequences that will be loaded on
 * us4oem:0, us4oem:1, etc. This function outputs updated sequences so that
 * there are no conflicting rx channels. All of the output sequences have the
 * same length - e.g. if seqs[0] first tx/rx operation must be split into
 * 4 tx/rx ops, and seqs[1] first op must be split into 2 tx/rx ops only,
 * the second sequence will extended by NOP TxRxParameters.
 *
 * @param seqs tx/rx sequences to recalculate
 * @param mappings tx/rx us4oem mappings to apply - in order to determien
 * @return recalculated sequences,
 *         a mapping (module, input op index, rx channel) -> output frame number,
 *         a mapping (module, input op index, rx channel) -> output frame rx channel
 */
SplitResult
splitRxAperturesIfNecessary(const std::vector<TxRxParamsSequence> &seqs,
                            const std::vector<std::vector<uint8_t>> &mappings,
                            const std::unordered_map<Ordinal, std::vector<arrus::framework::NdArray>> &txDelayProfiles,
                            Ordinal frameMetadataOem);

}

#endif //ARRUS_CORE_DEVICES_US4R_COMMON_H
