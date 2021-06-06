let emu_lib = {
        $Emu: {
            BUFFER_SIZE : 1024,
            SAMPLERATE : 48000,
            DURATION : this.BUFFER_SIZE/this.SAMPLERATE,
            DELAY : 50 / 1000,
            process_audio: function(e){
                Emu.init_audio();


                let samples_ptr = Module.ccall('fetch_samples', 'number', [], []);
                let samples_array = new Float32Array(Module["HEAPF32"].buffer, samples_ptr, Emu.BUFFER_SIZE * 2);

                for(let i = 0; i < Emu.BUFFER_SIZE; i++){
                    e.outputBuffer.getChannelData(0)[i] = samples_array[i*2];
                    e.outputBuffer.getChannelData(1)[i] = samples_array[i*2 + 1];
                }
            },
            init_audio: async function() {
                if(!Emu.audio) {
                    Emu.audio = {
                        buffers:[new Float32Array(Emu.BUFFER_SIZE), new Float32Array(Emu.BUFFER_SIZE)],
                        ctx: new AudioContext({sampleRate:Emu.SAMPLERATE}),
                    };

                    // Emu.audio.processorNode = Emu.audio.ctx.createScriptProcessor(Emu.BUFFER_SIZE)
                    // Emu.audio.processorNode.onaudioprocess = Emu.process_audio;
                    // Emu.audio.processorNode.connect(Emu.audio.ctx.destination);

                    try{
                        await Emu.audio.ctx.audioWorklet.addModule('../emu-sound-processor.js');

                    } catch(e) {
                        console.log("Error addModule");
                        console.log(e);
                    }


                    Emu.audio.buffer = Emu.audio.ctx.createBuffer(2, Emu.BUFFER_SIZE, Emu.SAMPLERATE);
                }
            },
        },
        init_js_lib : async function() {
            Emu.instance = Module.get_emulator_instance();
            console.log(Emu.instance);
            // fetch_audio_samples = Module.cwrap('fetch_samples', 'void', [],[])
            let samples_ptr =  Module.ccall('fetch_samples', 'number', ['number'], [0])
            await Emu.init_audio();

            Emu.audio.workletNode = new AudioWorkletNode(Emu.audio.ctx, "emu-sound-processor", { processorOptions: { heap_buffer: HEAPF32.buffer, ptr:samples_ptr}, outputChannelCount: [2] });
            Emu.audio.workletNode.port.onmessage = (e) => {
                // console.log(e.data);
                Module.ccall('fetch_samples', 'number', ['number'], [e.data]);
                Emu.audio.workletNode.port.postMessage(e.data);
            };
            Emu.audio.workletNode.connect(Emu.audio.ctx.destination);
        },
        play_samples: async function(samples){
            if(!Emu.audio)
                await Emu.init_audio();
            

            if(Emu.audio.workletNode)
                Emu.audio.workletNode.port.postMessage(samples);
        },
        canvas_update: async function(pixels){
            var ctx = Module['canvas'].getContext('2d');
            ctx.canvas.height = 144*4;
            ctx.canvas.width = 160*4;;
            ctx.imageSmoothingEnabled =false;

            if(!this.emu_canvas){
                this.emu_canvas = document.createElement("canvas");
                emu_canvas.width = 160;
                emu_canvas.height = 144;
            }

            const pixel_array = new Uint8ClampedArray(HEAPU8.buffer, pixels, 160*144*4);
            var image_data = new ImageData(new Uint8ClampedArray(pixel_array), 160,144);
            emu_canvas.getContext('2d').putImageData(image_data,0,0);


            ctx.drawImage(emu_canvas, 0, 0, ctx.canvas.width, ctx.canvas.height);
        }
}

autoAddDeps(emu_lib, '$Emu');
mergeInto(LibraryManager.library, emu_lib);