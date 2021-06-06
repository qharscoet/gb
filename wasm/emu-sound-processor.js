class EmuSoundWorklet extends AudioWorkletProcessor {
  constructor(options) {
    super();

    this.EMU_BUFFER_SIZE = 1024;
    this.BUFFER_SIZE = this.EMU_BUFFER_SIZE * 4;

    this.buffers = [new Float32Array(this.BUFFER_SIZE), new Float32Array(this.BUFFER_SIZE)];
    this.writePos = 0;
    this.readPos = 0;
    this.available = 0;
    this.heap_buffer = options.processorOptions.heap_buffer;
    this.samples_ptr = options.processorOptions.ptr;

    this.port.onmessage = (e) => {
      const len = e.data;
      var samples_fbuffer = new Float32Array(this.heap_buffer, this.samples_ptr, len * 2);

      for (var i = 0; i < len; i++) {
        this.buffers[0][this.writePos + i] = samples_fbuffer[i * 2];
        this.buffers[1][this.writePos + i] = samples_fbuffer[i * 2 + 1];
      }

      this.writePos += len;
      this.writePos %= this.BUFFER_SIZE;

      this.available += len;
    }
  }

  process(inputList, outputList, parameters) {
    /* using the inputs (or not, as needed), write the output
       into each of the outputs */
    const output = outputList[0];
    const len = output[0].length;
    this.port.postMessage(len);

    if (this.available > output[0].length) {
      for (let i = 0; i < output[0].length; i++) {
        output[0][i] = this.buffers[0][this.readPos + i];
        output[1][i] = this.buffers[1][this.readPos + i];
      }

      this.readPos += output[0].length;
      this.readPos %= this.BUFFER_SIZE;

      this.available -= output[0].length;
      if (this.available < 0)
        this.available = 0;
    }

    return true

  }
};

registerProcessor("emu-sound-processor", EmuSoundWorklet);