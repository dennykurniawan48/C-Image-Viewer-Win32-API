#include <Windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>

#pragma comment(lib, "Gdiplus.lib")

std::vector<std::string> listImages;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void getImagesOnCurrentDir() {
	std::string pattern = ".";
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	char fileName[FILENAME_MAX];
	GetCurrentDirectory(FILENAME_MAX, fileName);
	strcat_s(fileName, pattern.c_str());
	HANDLE hFind;
	if ((hFind = FindFirstFile(fileName, &data)) != INVALID_HANDLE_VALUE) {
		do {
			std::string nameFile = data.cFileName;
			size_t extloc = nameFile.rfind(".", nameFile.length());
			if (extloc != std::string::npos) {
				std::string filteredFile = nameFile.substr(extloc + 1, nameFile.length() - 1);
				if (filteredFile == "jpg" || filteredFile == "JPG" || filteredFile == "PNG" || filteredFile == "png") {
					listImages.push_back(data.cFileName);
				}
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	static const char ClassName[] = "MyImageViewer";
	WNDCLASSEX wc;
	MSG msg;
	HWND hwnd;

	Gdiplus::GdiplusStartupInput gdiStartup;
	ULONG_PTR gdiToken;

	Gdiplus::GdiplusStartup(&gdiToken, &gdiStartup, NULL);

	wc.cbClsExtra = 0;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = ClassName;
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Error register class", "Error", MB_ICONERROR);
		return 0;
	}

	getImagesOnCurrentDir();

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, ClassName, "Image Viewer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Error create window", "Error", MB_ICONERROR);
		return 0;
	}

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Gdiplus::GdiplusShutdown(gdiToken);
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	static unsigned int cxClient, cyClient, positionX, positionY, counter;
	switch (msg) {
		case WM_SIZE: {
			cxClient = LOWORD(lParam);
			cyClient = HIWORD(lParam);
			break;
		}
		case WM_KEYDOWN: {
			switch (wParam) {
				case VK_LEFT: { // keyboard arrow left
					if (counter > 0)
						counter--;
					else
						counter = listImages.size() - 1; // if user press left arrow key when beginning opening program then its open last images
					InvalidateRect(hwnd, NULL, TRUE);
					break;
				}
				case VK_RIGHT: {
					if (counter < (listImages.size() - 1))
						counter++;
					else
						counter = 0; // if user press right arrow when last image opened, its back to first file
					InvalidateRect(hwnd, NULL, TRUE);
					break;
				}
			}
			break;
		}
		case WM_PAINT: {
			hdc = BeginPaint(hwnd, &ps);
			Gdiplus::Graphics graph(hdc);
			std::wstring openedFile;

			if (listImages.size() == 0) { // no file detected
				// Show text images not found
				Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0));
				Gdiplus::FontFamily ff(L"Times New Roman");
				Gdiplus::Font font(&ff, 15, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				Gdiplus::PointF point(cxClient / 2, cyClient / 2);
				graph.DrawString(L"Images can't be found", -1, &font, point, &brush);
				break;
			}

			openedFile = std::wstring(listImages[counter].begin(), listImages[counter].end());

			Gdiplus::Image img(openedFile.c_str());
			static int CtrWidth, CtrHeight;

			if (img.GetWidth() <= cxClient && img.GetHeight() <= cyClient) {
				cxClient = img.GetWidth();
				cyClient = img.GetHeight();
			}
			else {
				double rasio;
				if (img.GetWidth() == img.GetHeight()) {
					if (cxClient >= cyClient)
						rasio = (double)img.GetHeight() / (double)cyClient;
					else
						rasio = (double)img.GetWidth() / (double)cxClient;
					CtrWidth = (double)img.GetWidth() / rasio;
					CtrHeight = (double)img.GetHeight() / rasio;
				}
				else if (img.GetWidth() > img.GetHeight()) {
					if (cxClient > cyClient)
						rasio = (double)img.GetWidth() / (double)cxClient;
					else
						rasio = (double)img.GetHeight() / (double)cyClient;
					CtrWidth = (double)img.GetWidth() / rasio;
					CtrHeight = (double)img.GetHeight() / rasio;
				}
				else {
					if (cyClient >= cxClient)
						rasio = (double)img.GetWidth() / (double)cxClient;
					else
						rasio = (double)img.GetHeight() / (double)cyClient;
					CtrWidth = (double)img.GetWidth() / rasio;
					CtrHeight = (double)img.GetHeight() / rasio;
				}
			}

			positionX = (cxClient - CtrWidth) / 2;
			positionY = (cyClient - CtrHeight) / 2;

			graph.DrawImage(&img, positionX, positionY, CtrWidth, CtrHeight);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_CLOSE: {
			DestroyWindow(hwnd);
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}