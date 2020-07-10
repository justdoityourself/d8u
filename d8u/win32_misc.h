#pragma once

namespace util
{
	namespace win32
	{
		/*void WindowsUserEncrypt(const Memory & m, Memory & r)
		{
			DATA_BLOB di;
			DATA_BLOB dou;

			di.pbData = m.data();
			di.cbData = m.size();

			if(!CryptProtectData(&di,NULL,NULL,NULL,NULL,0,&dou))
				 throw "Windows User Encryption Failure.";

			r.Create(dou.cbData);
			std::memcpy(r.data(),dou.pbData,r.size());

			LocalFree(dou.pbData);
		}

		void WindowsUserDecrypt(const Memory & m, Memory & r)
		{
			DATA_BLOB di;
			DATA_BLOB dou;

			di.pbData = m.data();
			di.cbData = m.size();

			if (!CryptUnprotectData(&di,NULL,NULL,NULL, NULL, 0,&dou))
				 throw "Windows User Decryption Failure.";

			r.Create(dou.cbData);
			std::memcpy(r.data(),dou.pbData,r.size());

			LocalFree(dou.pbData);
		}*/

		/*
			SERVICE_STATUS        g_ServiceStatus = {0};
			SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
			#define SERVICE_NAME "atk"

			VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
			{
				switch (CtrlCode)
				{
				 case SERVICE_CONTROL_STOP :

					if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
					{
						std::cout << "Service already stopping." << std::endl;
						break;
					}

					g_ServiceStatus.dwControlsAccepted = 0;
					g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
					g_ServiceStatus.dwWin32ExitCode = 0;
					g_ServiceStatus.dwCheckPoint = 4;

					if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
						std::cout << "ServiceCtrlHandler: SetServiceStatus returned error. " << GetLastError() << std::endl;

					std::cout << "Sending stop signal." << std::endl << std::flush;
					Foundation::make_singleton<Foundation::Management>().Stop();

					break;

				 default:
					 break;
				}
			}

			VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
			{
				std::string wd;
				DWORD copied = 0;
				do {
					wd.resize(wd.size()+MAX_PATH);
					copied = GetModuleFileName(0, (LPSTR)wd.data(), (DWORD)wd.size());
				} while( copied >= wd.size() );

				std::cout << "Working directory: " << Foundation::File::Split(wd).first << std::endl;
				SetCurrentDirectory(Foundation::File::Split(wd).first.c_str());

				DWORD Status = E_FAIL;

				freopen("atk.log","a",stdout);

				g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

				if (g_StatusHandle == NULL)
				{
					std::cout << "Failed RegisterServiceCtrlHandler. " << GetLastError() << std::endl;
					goto EXIT;
				}

				ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
				g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
				g_ServiceStatus.dwControlsAccepted = 0;
				g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
				g_ServiceStatus.dwWin32ExitCode = 0;
				g_ServiceStatus.dwServiceSpecificExitCode = 0;
				g_ServiceStatus.dwCheckPoint = 0;

				if (SetServiceStatus (g_StatusHandle , &g_ServiceStatus) == FALSE)
					std::cout << "ServiceMain: SetServiceStatus returned error. " << GetLastError() << std::endl;

				g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
				g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
				g_ServiceStatus.dwWin32ExitCode = 0;
				g_ServiceStatus.dwCheckPoint = 0;

				if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
					std::cout << "ServiceMain: SetServiceStatus returned error. " << GetLastError() << std::endl;

				UsingFoundation
				UsingDefault

				Foundation::Main(0,nullptr,true);

				std::cout << "Service control released." << std::endl;

				g_ServiceStatus.dwControlsAccepted = 0;
				g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
				g_ServiceStatus.dwWin32ExitCode = 0;
				g_ServiceStatus.dwCheckPoint = 3;

				if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
					std::cout << "ServiceMain: SetServiceStatus returned error. " << GetLastError() << std::endl;

			EXIT:
				return;
			}

			void InstallServiceIf(const std::string& name, const std::string& path)
			{
				SC_HANDLE hServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ;
				if (NULL != hServiceControlManager)
				{
					SC_HANDLE hFind = OpenService(hServiceControlManager, name.c_str(),SC_MANAGER_ALL_ACCESS);

					if(!hFind)
					{
						std::cout << "Created service." << std::endl;
						hFind = CreateService(hServiceControlManager,
												(LPCTSTR) name.c_str(),
												(LPCTSTR) name.c_str(),
												SC_MANAGER_ALL_ACCESS,
												SERVICE_WIN32_OWN_PROCESS,
												SERVICE_AUTO_START,
												SERVICE_ERROR_NORMAL,
												(LPCTSTR) path.c_str(),
												0,
												0,
												0,
												0,
												0) ;

						SERVICE_DESCRIPTION description = { "The ATK engine service. Provides the D8FS process." };
						ChangeServiceConfig2(hFind, SERVICE_CONFIG_DESCRIPTION, &description);
					}

					if(hFind)
					{
						DWORD out;
						SERVICE_STATUS_PROCESS sc;

						if(!QueryServiceStatusEx(hFind,SC_STATUS_PROCESS_INFO,(LPBYTE)&sc,sizeof(SERVICE_STATUS_PROCESS),&out))
							std::cout << "Failed to query service status." << std::endl;

						if(sc.dwCurrentState != SERVICE_RUNNING)
						{
							std::cout << "Service not running, attempting start..." << std::endl;
							if(!StartService(hFind,0,nullptr))
								std::cout << "Failed to start service. " << GetLastError() << std::endl;
							else
								std::cout << "Started Service." << std::endl;
						}
						else
							std::cout << "Service already running." << std::endl;

						CloseServiceHandle(hFind) ;
					}
					else
						std::cout << "Failed to create service." << std::endl;

					CloseServiceHandle(hServiceControlManager) ;
				}
				else
					std::cout << "Failed to open service management." << std::endl;
			}
		*/

		/*int main(int c, char ** a)
		{
			if(c >= 2 && std::string("--not_a_service") == a[1])
				return common_main(0,nullptr);

			__crc32_init();

			freopen("cfg.log","a",stdout);

			InstallServiceIf(SERVICE_NAME, a[0]);

			SERVICE_TABLE_ENTRY ServiceTable[] = 
			{
				{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
				{NULL, NULL}
			};
 
			if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
			{
				return GetLastError ();
			}
 
			return 0;
		}*/
	}
}