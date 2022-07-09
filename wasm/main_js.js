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

}

function update_hqx(size) {
	Emu.events.size_multiplier = size;
}

document.getElementById('uploader').addEventListener('change', transcode);
document.getElementById('save_button').addEventListener('click', get_save);
document.getElementById('hqx-select').addEventListener('change', (event) => update_hqx(event.target.value));
window.addEventListener('beforeunload', get_save);
display_save();