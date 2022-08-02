#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>

namespace vtfview
{
	class ImageImportDialog : public QDialog
	{
		Q_OBJECT;

	public:
		explicit ImageImportDialog( QWidget *pParent = nullptr );

	private:
		QTabWidget *tabWidget;
		QDialogButtonBox *buttonBox;
	};
} // namespace vtfview