const readFromBlobOrFile = (blob) => (
	new Promise((resolve, reject) => {
		const fileReader = new FileReader();
		fileReader.onload = () => {
			resolve(fileReader.result);
		};
		fileReader.onerror = ({ target: { error: { code } } }) => {
			reject(Error(`File could not be read! Code=${code}`));
		};
		fileReader.readAsArrayBuffer(blob);
	})
);


const transcode = async ({ target: { files } }) => {
	const { name } = files[0];
	// await ffmpeg.load();
	console.log(files);
	const file_array = await readFromBlobOrFile(files[0]);
	FS.writeFile(name, new Uint8Array(file_array));
	console.log(name);
	Module.ccall("wasm_load_file", null, ["string"], [name]);
	// await ffmpeg.run('-i', name, 'output.mp4');
	// const data = ffmpeg.FS('readFile', 'output.mp4');
	// const video = document.getElementById('player');
	// video.src = URL.createObjectURL(new Blob([data.buffer], { type: 'video/mp4' }));
}
document
	.getElementById('uploader').addEventListener('change', transcode);