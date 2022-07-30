
#include <QApplication>

#include <cstdlib>
#include <cstring>

#include "viewer.hpp"
#include "VTFLib.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	std::string file;
	if (argc > 1) {
		file = argv[1];
	}

	auto* pMainWindow = new vtfview::ViewerMainWindow(nullptr);
	if (file.length() && !pMainWindow->load_file(file.c_str())) {
		fprintf(stderr, "Could not open vtf '%s'!\n", file.c_str());
		return 1;
	}
	pMainWindow->show();

	return app.exec();
}
