#include <vcl.h>
#include <tchar.h>
#include <Data.DB.hpp>
#include <FireDAC.Comp.Client.hpp>
#include <FireDAC.Phys.hpp>
#include <FireDAC.Phys.Intf.hpp>
#include <FireDAC.Stan.Async.hpp>
#include <FireDAC.Stan.Def.hpp>
#include <FireDAC.Stan.Error.hpp>
#include <FireDAC.Stan.Intf.hpp>
#include <FireDAC.Stan.Option.hpp>
#include <FireDAC.Stan.Pool.hpp>
#include <FireDAC.UI.Intf.hpp>
#include <FireDAC.VCLUI.Wait.hpp>
#include <FireDAC.Phys.FB.hpp>
#include <FireDAC.Phys.FBDef.hpp>
#include <FireDAC.Comp.DataSet.hpp>
#include <FireDAC.DApt.hpp>
#include <FireDAC.DApt.Intf.hpp>
#include <FireDAC.DatS.hpp>
#include <FireDAC.Stan.Param.hpp>
#include <FireDAC.Phys.IBBase.hpp>
#include <Registry.hpp>
#include <boost/program_options.hpp>
#include <fstream>


namespace po = boost::program_options;

const String _regkey = "SOFTWARE\\Firebird Project\\Firebird Server\\Instances";

const std::string examples = R"(
Examples:
    hier beispiele
)";

class FirebirdInit
{
  public:
    FirebirdInit();
    String server_path;
    std::string path;
    String const _regkey;
};

int _tmain(int argc, _TCHAR* argv[])
{
        std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        // Verwenden Sie die TextConv-Funktionen, um TCHAR* in char* umzuwandeln
        const size_t size = wcslen(argv[i]) + 1;
        char* buffer = new char[size];
        wcstombs(buffer, argv[i], size);
        args.emplace_back(buffer);
        delete[] buffer;
    }

    po::options_description desc("Allowed options");
    desc.add_options()
		("help,h", "produce help message")
		("info","")
		("fdb,d",po::value<std::string>()->default_value("udc"),"firebird database or alias")
		("server,s",po::value<std::string>()->default_value("localhost"),"firebird server")
		("driverid",po::value<std::string>()->default_value("FB4"),"FDDRivers.ini ID")
		("user,u",po::value<std::string>()->default_value("sysdba"),"user")
		("password,p",po::value<std::string>()->default_value("masterkey"),"password")
		;

    po::options_description example(examples);
	po::options_description all("Usage");
	all.add(desc).add(example);
    po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, all), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

	if (vm.count("fdb")) {
		TFDConnection* FDConnection1;
		TFDTransaction* FDTransaction1;
		TFDQuery* FDQuery1;
		FDConnection1 = new TFDConnection(NULL);
		FDConnection1->Params->Clear();
		FDConnection1->LoginPrompt = false;
		FDConnection1->Params->DriverID = vm["driverid"].as<std::string>().c_str();
		FDConnection1->Params->Add("User_Name="+String(vm["user"].as<std::string>().c_str()));
		FDConnection1->Params->Add("Password="+String(vm["password"].as<std::string>().c_str()));
		FDConnection1->Params->Add("Database="+String(vm["fdb"].as<std::string>().c_str()));
		FDConnection1->Params->Add("Protocol=TCPIP");
		FDConnection1->Params->Add("Server="+String(vm["server"].as<std::string>().c_str()));
		std::cout << std::string(80, '=') << std::endl;

		for (int i{0}; i < FDConnection1->Params->Count; i++) {
		  String paramName = FDConnection1->Params->Names[i];
		  String paramValue = FDConnection1->Params->Values[paramName];
		  std::wcout << paramName << " " << paramValue << std::endl;
		}

        std::cout << std::string(80, '=') << std::endl;
		FDConnection1->Connected = true;
		FDQuery1 = new TFDQuery(NULL);
		FDQuery1->SQL->Add("select * from daten");
		FDQuery1->Connection = FDConnection1;
		FDQuery1->Open();
		FDQuery1->First();

		while (!FDQuery1->Eof) {
			std::wcout << FDQuery1->FieldByName("TEXT")->AsString << std::endl;
			FDQuery1->Next();
		}
		std::cout << FDQuery1->RecordCount;
		FDQuery1->Close();

		//		std::cout << "Compression level was set to "
		//				  << vm["compression"].as<int>() << ".\n";
    }
    else {
	   // std::cout << "Compression level was not set.\n";
    }



	FirebirdInit FBInst;
	std::wcout << "\nFirebirdServerDefaultInstance = " << FBInst.server_path << std::endl;
//	if (argc == 2 && _tcscmp(argv[1], _T("-h")) == 0) {
//		std::cout << "Auswahl:\n\n";
//		std::cout << "\t(w)echseln (cd)\n";
//        std::cin.get();
//	}
#ifdef _DEBUG
	std::cin.get();
#endif
  return 0;
}

bool __fastcall is_FirebirdInstalled()
{
    TRegistry* Registry = new TRegistry(KEY_READ);
    Registry->RootKey = HKEY_LOCAL_MACHINE;
    if (!Registry->KeyExists(
            "SYSTEM\\CurrentControlSet\\Services\\FirebirdServerDefaultInstance"))
    {
        return false;
    } else {
        return true;
    }
}

FirebirdInit::FirebirdInit() :
    server_path { "" }, path { "" }, _regkey {
        "SYSTEM\\CurrentControlSet\\Services\\FirebirdServerDefaultInstance"
    }
{
	TRegistry* Registry = new TRegistry(KEY_READ);
    Registry->RootKey = HKEY_LOCAL_MACHINE;
    if (Registry->KeyExists(_regkey)) {
        bool openResult = Registry->OpenKey(_regkey, true);
        if (!openResult) {
            return;
        }
		server_path = StringReplace(ExtractFilePath(Registry->ReadString("ImagePath")),"\\bin","", TReplaceFlags() << rfReplaceAll);
		String databases_conf { "" };
        if (FileExists(server_path + "aliases.conf")) {
            databases_conf = server_path + "aliases.conf";
        } else if (FileExists(server_path + "databases.conf")) {
            databases_conf = server_path + "databases.conf";
        }
        std::ifstream dbc(databases_conf.c_str());
        if (dbc) {
            std::string s { "" };
            while (dbc >> s) {
                if (s == "udc") {
                    dbc >> s;
                    dbc >> s;
                    break;
                }
            }
            std::replace(s.begin(), s.end(), '/', '\\');
			path = s.c_str();
        }
    }
}


