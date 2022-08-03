#include "imageImportDialog.hpp"

#include <QVBoxLayout>

using namespace vtfview;

ImageImportDialog::ImageImportDialog(QWidget* pParent)
	: QDialog(pParent) {
	auto* tabs = new QTabWidget;
	tabs->addTab(new QWidget, "General");
	tabs->addTab(new QWidget, "Advanced");
	tabs->addTab(new QWidget, "Resources");

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ImageImportDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &ImageImportDialog::reject);

	auto* layout = new QVBoxLayout;
	layout->addWidget(tabs);
	layout->addWidget(buttonBox);
	setLayout(layout);

	setWindowTitle("VTF Options");
}
