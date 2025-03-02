#include "mineBlasters.hpp"

static std::string		getFormattedTime()
{
	auto	now = std::chrono::system_clock::now();
	auto	nowTrunc = std::chrono::time_point_cast<std::chrono::seconds>(now);

	std::string format = std::format("{:%Y.%m.%d_%H.%M.%S}", nowTrunc);
	return format;
}

static void		insertData(const GameInfo& game, std::ofstream& out)
{
	out	<< "Level: " << game.misc.level << "\n"
		<< "Seed: " << game.misc.seed << "\n\n"
		
		<< "*Player Stats*" << "\n"
		<< "Coins: " << game.player.coinCount << "\n"
		<< "HP: " << game.player.health << "\n"
		<< "Max HP: " << game.player.maxHealth << "\n\n"

		<< "*Weapons*" << "\n"
		<< "Primary index: " << (game.player.primary.ptr == nullptr ? -1 : (game.player.primary.ptr - &game.weapons.all[0])) << "\n"
		<< "Primary ammo: " << game.player.primary.ammo << "\n"
		<< "Secondary index: " << (game.player.secondary.ptr == nullptr ? -1 : (game.player.secondary.ptr - &game.weapons.all[0])) << "\n"
		<< "Secondary ammo: " << game.player.secondary.ammo << "\n\n"

		<< "*Reserve Ammo*" << "\n" 
		<< "Light: " << game.player.reserveAmmo[0] << "\n"
		<< "Medium: " << game.player.reserveAmmo[1] << "\n"
		<< "Large: " << game.player.reserveAmmo[2] << "\n"
		<< "Shotgun: " << game.player.reserveAmmo[3] << "\n\n"

		<< "*Stats*" << "\n"
		<< "Ticks alive: "      << game.misc.MS - game.stats.startTime << "\n"
		<< "Coins collected: " << game.stats.coinsCollected << "\n"
		<< "Shots fired: "     << game.stats.shotsFired << "\n"
		<< "Damage dealt: "    << game.stats.damageDealt << "\n"
		<< "Enemies killed: "  << game.stats.enemiesKilled << "\n";
}

void	saveGame(const GameInfo& game)
{
	namespace fs = std::filesystem;

	std::string		folderPath = "saves";
	std::string		filePath = folderPath + "/" + getFormattedTime() + ".mbs";
	std::ofstream	out;

	if (fs::exists(folderPath) == false)
	{
		fs::create_directory(folderPath);
	}
	out.open(filePath);
	if (out.is_open() == false)
	{
		return;
	}
	insertData(game, out);
	out.close();
}

// --------------------------------------- ^^^ Save ^^^ ---------------------------------------- //
// ======================================                ======================================= //
// --------------------------------------- vvv Load vvv ---------------------------------------- //

static std::string	selectFile()
{
	namespace fs = std::filesystem;

	OPENFILENAMEA		ofn{};				// common dialog box structure
	char				szFile[260]{};		// buffer for file name
	std::u8string		saveFolderu8 = (fs::current_path() / "saves\\").u8string();
	std::string			saveFolder(saveFolderu8.begin(), saveFolderu8.end());
	std::string			result = "";

	ShowCursor(true);
	if (fs::exists(saveFolder) == false)
	{
		fs::create_directory(saveFolder);
	}
	ofn.lpstrInitialDir = saveFolder.c_str();
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;										// Handle to the owner window
	ofn.lpstrFile = (LPSTR)szFile;									// Pointer to file name buffer
	ofn.lpstrFile[0] = '\0';										// Initialize the file buffer
	ofn.nMaxFile = sizeof(szFile);									// Set the max file size
	ofn.lpstrFilter = (LPCSTR)"MBS Files\0*.MBS\0All Files\0*.*\0"; // Filter for .mbs files
	ofn.nFilterIndex = 1;											// Default filter to .mbs files
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = (LPCSTR)"Select a save file:";					// Dialog box title
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;				// Restrict selection to existing files

	if (GetOpenFileNameA(&ofn) == true)
	{
		result = (char*)ofn.lpstrFile;
	}
	ShowCursor(false);
	return result;
}

// T should be some type of integer
template <typename T>
static void		loadLine(std::string all, std::string key, T& val)
{
	size_t		start = all.find(key);
	size_t		len = all.find_first_of('\n\0', start);
	
	if (start != std::string::npos && len != std::string::npos)
	{
		start += key.length();
		std::string substr = all.substr(start, len);
		val = (T)std::stol(substr);
	}
}

void	loadContent(GameInfo& game, std::ifstream& in)
{
	std::string	content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	int32_t		temp;

	//resetLevel(game);
	game.state = GameState::PLAYING;
	game.misc.isInShop = false;

	loadLine(content, "Level: ",  game.misc.level);
	loadLine(content, "Seed: ",   game.misc.seed);
	loadLine(content, "Coins: ",  game.player.coinCount);
	loadLine(content, "HP: ",     game.player.health);
	loadLine(content, "Max HP: ", game.player.maxHealth);

	loadLine(content, "Primary index: ", temp);
	game.player.primary.ptr = (temp == -1 ? nullptr : &game.weapons.all[0] + temp);
	loadLine(content, "Primary ammo: ", game.player.primary.ammo);
	loadLine(content, "Secondary index: ", temp);
	game.player.secondary.ptr = (temp == -1 ? nullptr : &game.weapons.all[0] + temp);
	loadLine(content, "Secondary ammo: ", game.player.secondary.ammo);

	loadLine(content, "Light: "  , game.player.reserveAmmo[0]);
	loadLine(content, "Medium: " , game.player.reserveAmmo[1]);
	loadLine(content, "Large: "  , game.player.reserveAmmo[2]);
	loadLine(content, "Shotgun: ", game.player.reserveAmmo[3]);

	loadLine(content, "Ticks alive: "    , temp);
	game.stats.startTime = game.misc.MS - temp;
	loadLine(content, "Coins collected: ", game.stats.coinsCollected);
	loadLine(content, "Shots fired: "    , game.stats.shotsFired);
	loadLine(content, "Damage dealt: "   , game.stats.damageDealt);
	loadLine(content, "Enemies killed: " , game.stats.enemiesKilled);

	resetLevel(game);
}

void	loadGame(GameInfo& game)
{
	std::string		file = selectFile();
	std::ifstream	in;

	if (file == "")
		return;
	in.open(file);
	if (in.is_open() == false)
		return;
	loadContent(game, in);
}