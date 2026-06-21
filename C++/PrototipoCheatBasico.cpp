#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <variant>


DWORD GetPid(const wchar_t* procName)
{
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) 
        return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, procName) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return pid;
}

uintptr_t GetProcBaseAddress(DWORD pid)
{
    uintptr_t baseAddress = NULL;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnapshot == INVALID_HANDLE_VALUE) 
        return NULL;

    MODULEENTRY32 mod;
    mod.dwSize = sizeof(MODULEENTRY32);
    if (Module32First(hSnapshot, &mod)) 
        baseAddress = (uintptr_t) mod.modBaseAddr;

    CloseHandle(hSnapshot);
    return baseAddress;
}

template <typename T>
T ReadMemoryValue(HANDLE hProcess, LPVOID lpBaseAddress)
{
	T value = T{};
	ReadProcessMemory(hProcess, lpBaseAddress, &value, sizeof(value), nullptr);
	return value;
}

std::string ReadMemoryString(HANDLE hProcess, LPVOID lpBaseAddress, size_t sizeStirng)
{
	std::string str = std::string(sizeStirng, '\0');
	if(!ReadProcessMemory(hProcess, lpBaseAddress, (LPVOID)str.data(), str.size(), nullptr)) return "";
	return str;
}

std::string ReadMemoryBytes(HANDLE hProcess, LPVOID lpBaseAddress, size_t sizeBytes)
{
	std::vector<BYTE> bytes = std::vector<BYTE>(sizeBytes);
	if (!ReadProcessMemory(hProcess, lpBaseAddress, bytes.data(), sizeBytes, nullptr)) return "";
	
	std::ostringstream oss;
	oss << std::hex << std::uppercase << std::setfill('0') << std::setfill('0');

	for (size_t i = 0; i < sizeBytes; i++)
	{
		oss << std::setw(2) << static_cast<int>(bytes.at(i));
		if (i < sizeBytes - 1) oss << '|';
	}
	return oss.str();
}

template <typename T>
bool WriteMemoryValue(HANDLE hProcess, LPVOID lpBaseAddress, T value)
{
	T _value = value;
	return WriteProcessMemory(hProcess, lpBaseAddress, &_value, sizeof(_value), nullptr);
}

bool WriteMemoryString(HANDLE hProcess, LPVOID lpBaseAddress, const char* str)
{
	return WriteProcessMemory(hProcess, lpBaseAddress, str, strlen(str), nullptr);
}

bool WriteMemoryBytes(HANDLE hProcess, LPVOID lpBaseAddress, std::vector<BYTE> bytes)
{
	return WriteProcessMemory(hProcess, lpBaseAddress, bytes.data(), bytes.size(), nullptr);
}

bool KeyPressed(int vKey)
{
	bool pressed = false;
	while (GetAsyncKeyState(vKey) & 0x8000)
	{
		pressed = true;
		Sleep(100);
	}
	return pressed;
}

bool SetCurssorLinePosition(short linePos)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	COORD pos;
	pos.X = 0;
	pos.Y = linePos;

	return SetConsoleCursorPosition(hConsole, pos);
}

short GetCurssorLinePosition()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	return csbi.dwCursorPosition.Y;
}

void PrintRestoringTheLinePosition(short linePos, const char* msg)
{
	short oldLinePos = GetCurssorLinePosition();
	if (SetCurssorLinePosition(linePos))
	{
		std::cout << msg << std::endl;
		SetCurssorLinePosition(oldLinePos);
	}
}

enum VirtualKey
{
	A = 0x41,
	B = 0x42,
	C = 0x43,
	D = 0x44,
	E = 0x45,
	F = 0x46,
	G = 0x47,
	H = 0x48,
	I = 0x49,
	J = 0x4A,
	K = 0x4B,
	L = 0x4C,
	M = 0x4D,
	N = 0x4E,
	O = 0x4F,
	P = 0x50,
	Q = 0x51,
	R = 0x52,
	S = 0x53,
	T = 0x54,
	U = 0x55,
	V = 0x56,
	W = 0x57,
	X = 0x58,
	Y = 0x59,
	Z = 0x5A,

	Enter = 0x0D,
	Shift = 0x10,
	Control = 0x11,
	Alt = 0x12,
	Space = 0x20,
	//Escape = 0x1B,
	Tab = 0x09,
	Backspace = 0x08,
	CapsLock = 0x14,
	End = 0x23,
	Delete = 0x2E
};

const char* VirtualKeyToString(VirtualKey vKey)
{
	switch (vKey)
	{
	case A:          return "A";
	case B:          return "B";
	case C:          return "C";
	case D:          return "D";
	case E:          return "E";
	case F:          return "F";
	case G:          return "G";
	case H:          return "H";
	case I:          return "I";
	case J:          return "J";
	case K:          return "K";
	case L:          return "L";
	case M:          return "M";
	case N:          return "N";
	case O:          return "O";
	case P:          return "P";
	case Q:          return "Q";
	case R:          return "R";
	case S:          return "S";
	case T:          return "T";
	case U:          return "U";
	case V:          return "V";
	case W:          return "W";
	case X:          return "X";
	case Y:          return "Y";
	case Z:          return "Z";

	case Enter:      return "Enter";
	case Shift:      return "Shift";
	case Control:    return "Control";
	case Alt:        return "Alt";
	case Space:      return "Space";
	case Tab:        return "Tab";
	case Backspace:  return "Backspace";
	case CapsLock:   return "CapsLock";
	case End:        return "End";
	case Delete:     return "Delete";

	default:
		return "Desconhecida";
	}
}

class Game
{
private:
	HANDLE hGame = NULL;
	DWORD pid = NULL;
	uintptr_t baseAddress = NULL;
	const wchar_t* procName = nullptr;

	using Values = std::variant<bool, char, BYTE, short, WORD, int, DWORD, float, uintptr_t, const char*, std::vector<BYTE>>;
	size_t sizeValues[std::variant_size_v<Values>] = { sizeof(bool), sizeof(char), sizeof(BYTE), sizeof(short), sizeof(WORD), sizeof(int), sizeof(DWORD), sizeof(float), sizeof(uintptr_t) };

	enum ModificationType;

	struct Attribute
	{
		const char* name;
		bool modify;
		int indexType;
		short consolePosition;
		LPVOID address;
		DWORD oldProtect;
		size_t sizeValue;
		ModificationType mType;
		VirtualKey vKey;
		Values originalValue, modifiedValue;
		std::string currentValue, oldValue, vKeyStr;

		void Show(bool restoreLinePos)
		{

			std::ostringstream showString;
			showString << std::left << std::setw(18) << this->address << std::setw(20) << this->name << std::setw(13) << this->vKeyStr << std::setw(18) << this->currentValue;

			if (restoreLinePos) return PrintRestoringTheLinePosition(this->consolePosition, showString.str().c_str());

			std::cout << showString.str().c_str() << std::endl;
		}

		bool ChangedValue() { return this->currentValue != this->oldValue; }
	};

	std::vector<Attribute> attributes = std::vector<Attribute>();

	size_t GetAttributeValueSize(const Values value)
	{
		int index = value.index();
		switch (index)
		{
		case 9:
			return strlen(std::get<const char*>(value));
		case 10:
			return std::get<std::vector<BYTE>>(value).size();
		default:
			return sizeValues[index];
			break;
		}
	}

	LPVOID GetAttributeAddress(const std::vector<uintptr_t> offsets)
	{
		uintptr_t address = this->baseAddress;
		size_t size = offsets.size();

		for (size_t i = 0; i < size; i++)
		{
			address += offsets.at(i);
			if (i < size - 1) address = ReadMemoryValue<uintptr_t>(this->hGame, (LPVOID)address);
		}
		return (LPVOID)address;
	}

	std::string GetAttributeValue(const Attribute& attr)
	{
		switch (attr.indexType)
		{
		case 0:
			return ReadMemoryValue<bool>(this->hGame, attr.address) ? "True" : "False";
		case 1:
			return std::string(1, ReadMemoryValue<char>(this->hGame, attr.address));
		case 2:
			return std::to_string(ReadMemoryValue<BYTE>(this->hGame, attr.address));
		case 3:
			return std::to_string(ReadMemoryValue<short>(this->hGame, attr.address));
		case 4:
			return std::to_string(ReadMemoryValue<WORD>(this->hGame, attr.address));
		case 5:
			return std::to_string(ReadMemoryValue<int>(this->hGame, attr.address));
		case 6:
			return std::to_string(ReadMemoryValue<DWORD>(this->hGame, attr.address));
		case 7:
			return std::to_string(ReadMemoryValue<float>(this->hGame, attr.address));
		case 8:
			return std::to_string(ReadMemoryValue<uintptr_t>(this->hGame, attr.address));
		case 9:
			return ReadMemoryString(this->hGame, attr.address, attr.sizeValue);
		case 10:
			return ReadMemoryBytes(this->hGame, attr.address, attr.sizeValue);
		default:
			return "Tipo Invalido!";
			break;
		}
	}

	bool SetAttributeValue(const Attribute& attr, bool toggled = false)
	{

		Values value = toggled && !attr.modify ? attr.originalValue : attr.modifiedValue;

		switch (attr.indexType)
		{
		case 0:
			return WriteMemoryValue(this->hGame, attr.address, std::get<bool>(value));
		case 1:
			return WriteMemoryValue(this->hGame, attr.address, std::get<char>(value));
		case 2:
			return WriteMemoryValue(this->hGame, attr.address, std::get<BYTE>(value));
		case 3:
			return WriteMemoryValue(this->hGame, attr.address, std::get<short>(value));
		case 4:
			return WriteMemoryValue(this->hGame, attr.address, std::get<WORD>(value));
		case 5:
			return WriteMemoryValue(this->hGame, attr.address, std::get<int>(value));
		case 6:
			return WriteMemoryValue(this->hGame, attr.address, std::get<DWORD>(value));
		case 7:
			return WriteMemoryValue(this->hGame, attr.address, std::get<float>(value));
		case 8:
			return WriteMemoryValue(this->hGame, attr.address, std::get<uintptr_t>(value));
		case 9:
			return WriteMemoryString(this->hGame, attr.address, std::get<const char*>(value));
		case 10:
			return WriteMemoryBytes(this->hGame, attr.address, std::get<std::vector<BYTE>>(value));
		default:
			break;
		}
	}

	bool ModifyAttributePermissions(Attribute& attr, const DWORD flNewProtect)
	{
		return VirtualProtectEx(this->hGame, attr.address, attr.sizeValue, flNewProtect, &attr.oldProtect);
	}

	bool SetAttributeValueByKey(Attribute& attr)
	{
		if (KeyPressed(static_cast<int>(attr.vKey)))
		{
			attr.modify = !attr.modify;
			if (attr.mType == ModificationType::SingleModify && attr.ChangedValue()) return SetAttributeValue(attr);
			if (attr.mType == ModificationType::ToggleModify && attr.ChangedValue()) return SetAttributeValue(attr, true);
		}
		if (attr.mType == ModificationType::InfinityModify && attr.modify && attr.ChangedValue()) return SetAttributeValue(attr);

		return true;
	}

	bool RestoreAttribute(Attribute& attr)
	{
		attr.modify = false;
		bool restoreValue = SetAttributeValue(attr, true);
		return ModifyAttributePermissions(attr, attr.oldProtect) && restoreValue;
	}

public:
	enum ModificationType
	{
		SingleModify,
		ToggleModify,
		InfinityModify
	};

	const char* Name = nullptr;

	Game(const char* name, const wchar_t* procName)
	{
		this->Name = name;
		this->procName = procName;
		this->pid = GetPid(this->procName);
		this->baseAddress = GetProcBaseAddress(this->pid);
		this->hGame = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, false, this->pid);
	}

	~Game() { this->Dispose(); }

	HANDLE GetHandle() { return this->hGame; }

	uintptr_t GetBaseAddress() { return this->baseAddress; }

	DWORD GetPID() { return this->pid; }

	void GetProcName() { std::wcout << this->procName << std::endl; }

	bool IsRunning()
	{
		DWORD exitCode;
		return GetExitCodeProcess(this->hGame, &exitCode) && exitCode == STILL_ACTIVE;
	}

	bool AddAttribute(const char* name, std::vector<uintptr_t> offsets, ModificationType mtype, VirtualKey vkey, Values originalvalue, Values modifiedValue)
	{
		int indexType = originalvalue.index();
		if (indexType != modifiedValue.index()) return false;

		Attribute attr;
		attr.name = name;
		attr.mType = mtype;
		attr.vKey = vkey;
		attr.originalValue = originalvalue;
		attr.modifiedValue = modifiedValue;
		attr.indexType = indexType;
		attr.modify = false;
		attr.vKeyStr = VirtualKeyToString(attr.vKey);
		attr.address = GetAttributeAddress(offsets);
		attr.sizeValue = GetAttributeValueSize(attr.originalValue);
		bool modified = ModifyAttributePermissions(attr, PAGE_EXECUTE_READWRITE);
		if (modified) {
			attr.currentValue = GetAttributeValue(attr);
			attr.oldValue = "";
			attributes.emplace_back(attr);
		}
		return modified;
	}

	void Show()
	{

		std::cout << 
			"--------------------------------------------------------------------\n" <<
			"                         " << this->Name << "                       \n" <<
			"--------------------------------------------------------------------\n";
		std::wcout <<
			"Process Name: " << this->procName << std::endl;
		std::cout << 
			"PID: " << this->pid << std::endl <<
			"Handle: 0x" << this->hGame << std::endl <<
			"Base Address: 0x" << std::hex << this->baseAddress << std::endl << std::dec <<
			"--------------------------------------------------------------------\n" <<
			"----------------------------- ATRIBUTOS ----------------------------\n" <<
		std::left << std::setw(18) << "End." << std::setw(18) << "Nome." << std::setw(15) << "Tecla." << std::setw(18) << "Valor:" << std::right << std::endl;
		
		for (Attribute& attr : attributes)
		{
			attr.consolePosition = GetCurssorLinePosition();
			attr.Show(false);
		}

	}

	void ShowAttributes()
	{
		std::string currentValue = "";

		for (Attribute& attr : attributes)
		{
			if (!SetAttributeValueByKey(attr))
			{
				attr.currentValue = "Falha ao modificar valor deste atributo";
				return;
			}

			currentValue = GetAttributeValue(attr);
			if (currentValue != attr.currentValue)
			{
				attr.currentValue = currentValue;
				attr.Show(true);
			}

		}
	}

	bool Dispose()
	{
		bool disposed = true;
		if (this->IsRunning())
		{
			for (Attribute& attr : attributes)
			{
				if (!RestoreAttribute(attr)) disposed = false;
			}
		}
		return CloseHandle(this->hGame) && disposed;
	}

};

void DrawMessage(const char* msg, WORD color)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, color);

	std::cout << msg << std::endl;

	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

}

int main()
{
	Game game = Game("ASSAULT CUBE 1.3.0.2", L"ac_client.exe");

	if (!game.GetPID())
	{
		DrawMessage("+--------------------------------------+\n"
			        "|       PROCESSO NAO ENCONTRADO        |\n"
			        "+--------------------------------------+\n", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		return -1;
	}

	if (!game.GetHandle())
	{
		DrawMessage("+--------------------------------------+\n"
			        "|  FALHA AO OBTER HANDLE DO PROCESSO   |\n"
			        "+--------------------------------------+\n", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		return -2;
	}

	DrawMessage("+------------------------------------------------------------------+\n"
			    "|                                                                  |\n"
                "|            #####   #     #  #######    #    #######              |\n"
                "|           #     #  #     #  #         # #      #                 |\n"
                "|           #        #######  #####    #####     #                 |\n"
                "|           #        #     #  #       #     #    #                 |\n"
                "|           #     #  #     #  #       #     #    #                 |\n"
                "|            #####   #     #  ####### #     #    #                 |\n"
                "|                                                                  |\n"
                "+ -----------------------------------------------------------------+",FOREGROUND_RED);

	game.AddAttribute("Vida", std::vector<uintptr_t> {0x18AC00, 0xEC}, Game::ModificationType::ToggleModify, VirtualKey::V, 100, 9999);
	game.AddAttribute("Municao Rifle", std::vector<uintptr_t> {0x18AC00, 0x140}, Game::ModificationType::SingleModify, VirtualKey::R, 30, 9999);
	game.AddAttribute("No Recoil", std::vector<uintptr_t> {0xC2EC3}, Game::ModificationType::ToggleModify, VirtualKey::N, std::vector<BYTE> { 0xF3, 0x0F, 0x11, 0x56, 0x38}, std::vector<BYTE> {0x90, 0x90, 0x90, 0x90, 0x90});
	game.AddAttribute("Nome", std::vector<uintptr_t> {0x18AC00, 0x205}, Game::ModificationType::ToggleModify, VirtualKey::A, "username", "Luan    ");

	game.Show();

	while (!KeyPressed(static_cast<int>(VirtualKey::End)) && game.IsRunning())
	{
		game.ShowAttributes();
		Sleep(100);
	}

	//game.Dispose();

	system("cls");

	DrawMessage("+--------------------------------------+\n"
				"|          CHEAT FINALIZADO            |\n"
                "+--------------------------------------+\n", FOREGROUND_GREEN | FOREGROUND_INTENSITY);


	
}

