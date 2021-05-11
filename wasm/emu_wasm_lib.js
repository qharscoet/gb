let emu_lib = {
    canvas_update: function(pixels){
        var ctx = canvas.getContext('2d');
        var image_data = new ImageData(new Uint8ClampedArray(HEAPU8.buffer, pixels, 160*144*4), 160,144);
        ctx.putImageData(image_data, 0,0);
    }
}


mergeInto(LibraryManager.library, emu_lib);