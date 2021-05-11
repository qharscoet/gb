let emu_lib = {
    canvas_update: async function(pixels){
        var ctx = Module['canvas'].getContext('2d');
        ctx.canvas.height = 144*4;
        ctx.canvas.width = 160*4;;
        ctx.imageSmoothingEnabled =false;

        let emu_canvas = document.createElement("canvas");
        emu_canvas.width = 160;
        emu_canvas.height = 144;
        const pixel_array = new Uint8ClampedArray(HEAPU8.buffer, pixels, 160*144*4);
        var image_data = new ImageData(new Uint8ClampedArray(pixel_array), 160,144);
        emu_canvas.getContext('2d').putImageData(image_data,0,0);


        ctx.drawImage(emu_canvas, 0, 0, ctx.canvas.width, ctx.canvas.height);
    },
    play_samples: function(samples){
        const BUFFER_SIZE = 1024
        var ctx = new AudioContext({sampleRate:48000});
        var samples_fbuffer = new Float32Array(HEAPF32.buffer, samples, BUFFER_SIZE * 2)
        var ctx_buffer = ctx.createBuffer(2, BUFFER_SIZE, 48000);
        let left = ctx_buffer.getChannelData(0);
        let right = ctx_buffer.getChannelData(1);
        for(var i = 0; i < BUFFER_SIZE; i++) {
            left[i] = samples_fbuffer[i*2];
            right[i] = samples_fbuffer[i*2 +1];
        }

        source = ctx.createBufferSource();
        source.buffer = ctx_buffer;
        source.connect(ctx.destination);
        source.start();
    }
}


mergeInto(LibraryManager.library, emu_lib);