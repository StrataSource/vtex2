#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>

namespace vtfview
{
	class ImageImportDialog : public QDialog {
		Q_OBJECT;

	public:
		explicit ImageImportDialog(QWidget* pParent = nullptr);

	private:
		QDialogButtonBox* buttonBox;
	};
} // namespace vtfview