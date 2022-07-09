let emu_lib = {
        $Emu: {
            audio:{
                BUFFER_SIZE : 1024,
                SAMPLERATE : 48000,
                DURATION : this.BUFFER_SIZE/this.SAMPLERATE,
                DELAY : 50 / 1000,
                init:false,
            },
            events:{
                keystate:0,
                mapped_keys:['c','x','Backspace', 'Enter', 'ArrowRight', 'ArrowLeft','ArrowUp','ArrowDown'],
                buttons_states: [false, false, false, false, false, false, false, false],
                size_multiplier: 1,
            },
            print: function(str) {
                console.log(str);
            },
            handle_events: function(e) {
                e.preventDefault();
                let down = e.type == "keydown";
                if(!e.repeat){
                    if(Emu.events.mapped_keys.includes(e.key))
                    {
                        Emu.events.buttons_states[Emu.events.mapped_keys.indexOf(e.key)] = down;

                    }

                    if(e.key >= 1 && e.key <= 4)
                        Emu.events.size_multiplier = parseInt(e.key);
                }
            },
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

            Emu.audio.analyser = Emu.audio.ctx.createAnalyser();
            Emu.audio.workletNode.connect(Emu.audio.analyser);
            Emu.audio.analyser.connect(Emu.audio.ctx.destination);

            Emu.audio.analyser.fftSize = 2048;
            var bufferLength = Emu.audio.analyser.fftSize;
            Emu.audio.analyse_data = new Uint8Array(bufferLength);
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
        },
        audio_visualizer_draw : function() {
            if(Emu.audio.analyser){

                const audio_canvas = document.getElementById("audio_canvas");
                const WIDTH = 500;
                const HEIGHT = 200;
                const bufferLength = Emu.audio.analyser.fftSize;

                audio_canvas.width = WIDTH;
                audio_canvas.height = HEIGHT;
                canvasCtx = audio_canvas.getContext('2d');
                Emu.audio.analyser.getByteTimeDomainData(Emu.audio.analyse_data);

                canvasCtx.fillStyle = 'rgb(200, 200, 200)';
                canvasCtx.fillRect(0, 0, WIDTH, HEIGHT);

                canvasCtx.lineWidth = 2;
                canvasCtx.strokeStyle = 'rgb(0, 0, 0)';

                canvasCtx.beginPath();

                var sliceWidth = WIDTH * 1.0 / bufferLength;
                var x = 0;

                for (var i = 0; i < bufferLength; i++) {

                    var v = Emu.audio.analyse_data[i] / 128.0;
                    var y = v * HEIGHT / 2;

                    if (i === 0) {
                        canvasCtx.moveTo(x, y);
                    } else {
                        canvasCtx.lineTo(x, y);
                    }

                    x += sliceWidth;
                }

                canvasCtx.lineTo(audio_canvas.width, audio_canvas.height / 2);
                canvasCtx.stroke();
        fetch_save: function(game_name_ptr) {
            const game_name = UTF8ToString(game_name_ptr);
            console.log("fetching save from" + game_name );
            if(localStorage.getItem(game_name))
            {
                console.log("found save");
                const data = localStorage.getItem(game_name);
                FS.writeFile(game_name + ".sav", Uint8Array.from(data.split(','), Number));
            }
        }
}

autoAddDeps(emu_lib, '$Emu');
mergeInto(LibraryManager.library, emu_lib);