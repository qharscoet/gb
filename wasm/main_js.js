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
	//Save before switching game
	get_save();
	Module.ccall("wasm_load_file", null, ["string"], [name]);
}
function get_save() {
	emu = Module.get_emulator_instance();
	const gamename = emu.get_game_name();

	//This is the default name that the emulator returns
	if(gamename !== "GameBoy Emulator")
	{
		const filename = gamename + ".sav";
		emu.save();
		console.log("saved");
		const savefile = FS.readFile(filename, {encoding:"binary"});
		localStorage.setItem(gamename, savefile);
		display_save();
	} else
	{
		console.log("no rom loaded");
	}
}

function display_save() {
	const table = document.querySelector("#local_saves tbody");
	table.innerHTML="";
	for (let i = 0; i < localStorage.length; i++) {
		const keyname = localStorage.key(i);

		const row = table.insertRow();
		row.insertCell().appendChild(document.createTextNode(keyname));

		const button = document.createElement("button");
		button.innerHTML = "Delete";
		button.onclick = () => {
			localStorage.removeItem(keyname);
			display_save();
		};

		row.insertCell().appendChild(button);
	}

	// await ffmpeg.run('-i', name, 'output.mp4');
	// const data = ffmpeg.FS('readFile', 'output.mp4');
	// const video = document.getElementById('player');
	// video.src = URL.createObjectURL(new Blob([data.buffer], { type: 'video/mp4' }));

}

document.getElementById('uploader').addEventListener('change', transcode);
document.getElementById('save_button').addEventListener('click', get_save);
window.addEventListener('beforeunload', get_save);
display_save();