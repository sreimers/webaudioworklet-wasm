class WorkletProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.WEBEAUDIO_FRAME_SIZE = 128;
        this.WEBEAUDIO_READ_SIZE = 960; // 20ms@48000 Hz

        this.port.onmessage = (e) => {
            if (e.data.cmd == 'wasm') {
                WebAssembly.instantiate(e.data.payload, {})
                .then((result) => {
                    const exports = result.instance.exports;

                    this.store = exports.magic_store;
                    this.copy = exports.magic_copy;
                    exports.magic_alloc(sampleRate);

                    this.writeBuffer = new Float32Array(exports.memory.buffer,
                                                         exports.magic_write(),
                                                         this.WEBEAUDIO_FRAME_SIZE);
                    this.readBuffer  = new Int16Array(exports.memory.buffer,
                                                         exports.magic_read(),
                                                         this.WEBEAUDIO_READ_SIZE);
                });
            }

            if (e.data.cmd == 'audio') {
                if (this.copy(e.data.payload, 0) < 0)
                    return;

                var data = [];
                this.readBuffer.forEach((e) => {data.push(e);})
                this.port.postMessage(data)
            }
        }
    }
    process(inputList, outputList, parameters) {   
        if (!this.writeBuffer)
            return true;

        this.writeBuffer.set(inputList[0][0]);
        console.log(this.store(currentTime));

        return true;
    }
}
registerProcessor('worklet-processor', WorkletProcessor);
