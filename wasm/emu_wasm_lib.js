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
    }
}


mergeInto(LibraryManager.library, emu_lib);