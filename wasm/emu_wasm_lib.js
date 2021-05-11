let emu_lib = {
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
        },
        play_samples: function(samples){
            const BUFFER_SIZE = 1024;
            const SAMPLERATE = 48000;
            const DURATION = BUFFER_SIZE/SAMPLERATE;
            const DELAY = 50 / 1000;

            if(!this.audio) this.audio = {};
            if(!this.audio.ctx)
                this.audio.ctx = new AudioContext({sampleRate:SAMPLERATE});

            if(!this.audio.buffer)
                this.audio.buffer = this.audio.ctx.createBuffer(2, BUFFER_SIZE, SAMPLERATE);

            if(!this.audio.nextPlayTime)
                this.audio.nextPlayTime = 0;

            var secsUntilNextPlayStart = this.audio.nextPlayTime - this.audio.ctx.currentTime;
            if (secsUntilNextPlayStart >= DELAY + DURATION) return;


            var samples_fbuffer = new Float32Array(HEAPF32.buffer, samples, BUFFER_SIZE * 2)
            let left = this.audio.buffer.getChannelData(0);
            let right = this.audio.buffer.getChannelData(1);
            for(var i = 0; i < BUFFER_SIZE; i++) {
                left[i] = samples_fbuffer[i*2];
                right[i] = samples_fbuffer[i*2 +1];
            }

            var playtime = Math.max(this.audio.ctx.currentTime + DELAY, this.audio.nextPlayTime);

            source = this.audio.ctx.createBufferSource();
            source.buffer = this.audio.buffer;
            source.connect(this.audio.ctx.destination);
            source.start(playtime);

            this.audio.nextPlayTime = playtime + DURATION;
        }
}


mergeInto(LibraryManager.library, emu_lib);