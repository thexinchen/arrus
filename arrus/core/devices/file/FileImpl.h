#ifndef ARRUS_CORE_DEVICES_FILE_FILEIMPL_H
#define ARRUS_CORE_DEVICES_FILE_FILEIMPL_H

#include <array>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#include "arrus/core/api/devices/File.h"
#include "arrus/core/api/devices/Ultrasound.h"
#include "arrus/core/api/framework/Buffer.h"
#include "arrus/core/api/session/Metadata.h"
#include "arrus/core/api/devices/FileSettings.h"
#include "arrus/core/api/framework/NdArray.h"
#include "arrus/core/devices/file/FileBuffer.h"
#include "arrus/core/devices/file/FileBufferElement.h"
#include "arrus/core/api/common/Parameters.h"

namespace arrus::devices {

class FileProbe: public Probe {
public:
    FileProbe(const DeviceId &id, ProbeModel model) : Probe(id), model(std::move(model)) {}
    const ProbeModel &getModel() const override { return model; }

private:
    ProbeModel model;
};

class FileImpl : public File {
public:
    enum class State { STARTED, STOPPED };

    ~FileImpl() override = default;
    FileImpl(const DeviceId &id, const FileSettings &settings);
    FileImpl(FileImpl const &) = delete;
    FileImpl(FileImpl const &&) = delete;

    void start() override;
    void stop() override;
    void trigger() override;
    float getSamplingFrequency() const override;
    float getCurrentSamplingFrequency() const override;
    std::pair<arrus::framework::Buffer::SharedHandle, arrus::session::Metadata::SharedHandle>
    upload(const ops::us4r::Scheme &scheme) override;
    Probe *getProbe(Ordinal ordinal) override;
    void setParameters(const Parameters &params) override;


private:
    using Frame = std::vector<int16_t>;
    std::vector<Frame> readDataset(const std::string &filepath);

    void producer();
    void consumer();

    State state{State::STOPPED};
    Logger::Handle logger;
    std::mutex deviceStateMutex;
    std::thread producerThread;
    std::thread consumerThread;
    FileSettings settings;
    std::vector<Frame> dataset;
    arrus::framework::NdArray::Shape frameShape;
    std::optional<ops::us4r::Scheme> currentScheme;
    float currentFs;
    std::shared_ptr<FileBuffer> buffer;
    std::unique_ptr<FileProbe> probe;

    std::mutex parametersMutex;
    std::optional<int> pendingSliceBegin;
    std::optional<int> pendingSliceEnd;
    int txBegin, txEnd;
};

}// namespace arrus::devices


#endif//ARRUS_CORE_DEVICES_FILE_FILEIMPL_H
