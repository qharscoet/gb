let emu_lib = {
        $Emu: {
            BUFFER_SIZE : 1024,
            SAMPLERATE : 48000,
            DURATION : this.BUFFER_SIZE/this.SAMPLERATE,
            DELAY : 50 / 1000,
            process_audio: function(e){
                if(!Emu.audio)
                    Emu.init_audio();

                for(let i = 0; i < Emu.BUFFER_SIZE; i++){
                    e.outputBuffer.getChannelData(0)[i] = Emu.audio.buffers[0][i];
                    e.outputBuffer.getChannelData(1)[i] = Emu.audio.buffers[1][i];
                }
            },
            init_audio: function() {
                if(!Emu.audio) {
                    Emu.audio = {
                        buffers:[new Float32Array(Emu.BUFFER_SIZE), new Float32Array(Emu.BUFFER_SIZE)],
                        ctx: new AudioContext({sampleRate:Emu.SAMPLERATE}),
                    };

                    Emu.audio.processorNode = Emu.audio.ctx.createScriptProcessor(Emu.BUFFER_SIZE)
                    Emu.audio.processorNode.onaudioprocess = Emu.process_audio;
                    Emu.audio.processorNode.connect(Emu.audio.ctx.destination);
                    Emu.audio.buffer = Emu.audio.ctx.createBuffer(2, Emu.BUFFER_SIZE, Emu.SAMPLERATE);
                }
            },
        },
        play_samples: function(samples){
            if(!Emu.audio)
                Emu.init_audio();
            
            // if(!this.audio.nextPlayTime)
            //     this.audio.nextPlayTime = 0;

            // var secsUntilNextPlayStart = this.audio.nextPlayTime - this.audio.ctx.currentTime;
            // if (secsUntilNextPlayStart >= DELAY + DURATION) return;


            var samples_fbuffer = new Float32Array(HEAPF32.buffer, samples, Emu.BUFFER_SIZE * 2)
            // let left = Emu.audio.buffer.getChannelData(0);
            // let right = Emu.audio.buffer.getChannelData(1);
            for(var i = 0; i < Emu.BUFFER_SIZE; i++) {
                Emu.audio.buffers[0][i] = samples_fbuffer[i*2];
                Emu.audio.buffers[1][i] = samples_fbuffer[i*2 +1];
                // left[i] = samples_fbuffer[i*2];
                // right[i] = samples_fbuffer[i*2 +1];
            }


            // var playtime = Math.max(this.audio.ctx.currentTime + DELAY, this.audio.nextPlayTime);

            // source = Emu.audio.ctx.createBufferSource();
            // source.buffer = Emu.audio.buffer;
            // source.connect(Emu.audio.ctx.destination);
            // source.start();

            // this.audio.nextPlayTime = playtime + DURATION;
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