
#include "viewer.hpp"

#include "common/enums.hpp"
#include "common/image.hpp"
#include "common/util.hpp"
#include "fmt/format.h"
#include "imageImportDialog.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStyle>
#include <QComboBox>
#include <QToolBar>

#include <iostream>

using namespace vtfview;
using namespace VTFLib;

//////////////////////////////////////////////////////////////////////////////////
// ViewerMainWindow
//////////////////////////////////////////////////////////////////////////////////

ViewerMainWindow::ViewerMainWindow(QWidget* pParent)
	: QMainWindow(pParent) {
	setup_ui();
}

void ViewerMainWindow::setup_ui() {
	setWindowTitle("VTFView");

	setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);
	setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);
// Create the doc
	doc_ = new Document(this);
	// Info widget
	auto* infoDock = new QDockWidget(tr("Info"), this);

	auto* infoWidget = new InfoWidget(this);

	infoDock->setWidget(infoWidget);
	addDockWidget(Qt::RightDockWidgetArea, infoDock);

	// Resource list
	auto* resDock = new QDockWidget(tr("Resources"), this);

	auto* resList = new ResourceWidget(this);

	resDock->setWidget(resList);
	addDockWidget(Qt::RightDockWidgetArea, resDock);

	// Main image viewer
	auto* scroller = new QScrollArea(this);
	viewer_ = new ImageViewWidget(this);
	scroller->setAlignment(Qt::AlignCenter);

	connect(
		document(), &Document::vtfFileChanged,
		[this](const std::string& path, VTFLib::CVTFFile* file)
		{
			viewer_->set_vtf(file);
		});
	scroller->setVisible(true);
	scroller->setWidget(viewer_);

	setCentralWidget(scroller);

	// Viewer settings
	auto* viewerDock = new QDockWidget(tr("Viewer Settings"), this);

	auto* viewSettings = new ImageSettingsWidget(viewer_, this);
	connect(viewSettings, &ImageSettingsWidget::fileModified, this, &ViewerMainWindow::mark_modified);
// Hookup VTF change events
	connect(
		document(), &Document::vtfFileChanged,
		[this, viewSettings, resList, infoWidget](const std::string& path, VTFLib::CVTFFile* file)
		{
			viewSettings->set_vtf(file);
			resList->set_vtf(file);
			infoWidget->update_info(file);

			setWindowTitle(fmt::format(FMT_STRING("VTFView - [{}]"), str::get_filename(path.c_str())).c_str());
		});
	viewerDock->setWidget(viewSettings);
	addDockWidget(Qt::LeftDockWidgetArea, viewerDock);

	// Tabify the docks
	tabifyDockWidget(infoDock, resDock);

	// Setup the menu bars
	setup_menubar();
}

void ViewerMainWindow::setup_menubar() {
	auto* toolBar = new QToolBar(this);
	this->addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);

	toolBar->addAction(
		style()->standardIcon(QStyle::SP_FileIcon), "New File",
		[this]()
		{
			this->new_file();
		});
	toolBar->addAction(
		style()->standardIcon(QStyle::SP_DialogSaveButton), "Save File",
		[this]()
		{
			document()->save();
		});
	toolBar->addAction(
		style()->standardIcon(QStyle::SP_DialogOpenButton), "Open File",
		[this]()
		{
			this->open_file();
		});
	toolBar->addAction(
		style()->standardIcon(QStyle::SP_BrowserReload), "Reload File",
		[this]()
		{
			this->reload_file();
		});
	toolBar->addSeparator();
	toolBar->addAction(
		QIcon::fromTheme("zoom-in", QIcon(":/zoom-plus.svg")), "Zoom In",
		[this]()
		{
			viewer_->zoom(.1f);
		});
	toolBar->addAction(
		QIcon::fromTheme("zoom-out", QIcon(":/zoom-minus.svg")), "Zoom Out",
		[this]()
		{
			viewer_->zoom(-.1f);
		});
	// File menu
	auto* fileMenu = menuBar()->addMenu(tr("File"));
	fileMenu->addAction(
		style()->standardIcon(QStyle::SP_FileIcon), "New",
		[this]()
		{
			this->new_file();
		});
	fileMenu->addAction(
		style()->standardIcon(QStyle::SP_DialogOpenButton), "Open",
		[this]()
		{
			this->open_file();
		});
	fileMenu->addSeparator();
	fileMenu->addAction(
		style()->standardIcon(QStyle::SP_DialogSaveButton), "Save",
		[this]()
		{
			document()->save();
		});
	fileMenu->addAction(
		style()->standardIcon(QStyle::SP_DialogSaveButton), "Save As",
		[this]()
		{
			this->save(true);
		});
	fileMenu->addAction(
		style()->standardIcon(QStyle::SP_BrowserReload), "Reload File",
		[this]()
		{
			this->reload_file();
		});
	fileMenu->addAction(
		style()->standardIcon(QStyle::SP_ArrowUp), "Import File",
		[this]()
		{
			this->import_file();
		});
	fileMenu->addSeparator();
	fileMenu->addAction("Exit", [this]() {
		this->close();
	});

	// View menu
	auto* viewMenu = menuBar()->addMenu(tr("View"));
	viewMenu->addAction(QIcon::fromTheme("zoom-in", QIcon(":/zoom-plus.svg")), "Zoom In", [this]() {
		viewer_->zoom(.1f);
	}, QKeySequence::ZoomIn);
	viewMenu->addAction(QIcon::fromTheme("zoom-out", QIcon(":/zoom-minus.svg")), "Zoom Out", [this]() {
		viewer_->zoom(-.1f);
	}, QKeySequence::ZoomOut);

	// Help menu
	auto* helpMenu = menuBar()->addMenu(tr("Help"));
	helpMenu->addAction(
		"About",
		[this]()
		{
			QMessageBox::about(
				this, tr("About"),
				"VTFView & vtex2 by Chaos Initiative\n\nBuilt using VTFLib by Neil 'Jed' Jedrzejewski & Ryan Gregg, "
				"modified by Joshua Ashton");
		});
	helpMenu->addAction(
		"About Qt",
		[]()
		{
			qApp->aboutQt();
		});
}

void ViewerMainWindow::reset_state() {
	document()->unmark_modified();
}

void ViewerMainWindow::closeEvent(QCloseEvent* event) {
	if (document()->dirty()) {
		auto msgBox = new QMessageBox(
			QMessageBox::Icon::Question, tr("Quit without saving?"),
			tr("You have unsaved changes. Would you like to save?"), QMessageBox::NoButton, this);
		msgBox->addButton(QMessageBox::Save);
		msgBox->addButton(QMessageBox::Cancel);
		msgBox->addButton(QMessageBox::Close);
		auto r = msgBox->exec();

		if (r == QMessageBox::Cancel) {
			event->ignore(); // Just eat the event
			return;
		}
		else if (r == QMessageBox::Save) {
			document()->save();
		}
	}
}

void ViewerMainWindow::mark_modified() {
	auto title = windowTitle();
	if (title.endsWith("*"))
		return;
	setWindowTitle(title + "*");
	document()->mark_modified();
}

void ViewerMainWindow::unmark_modified() {
	// Clear out the window asterick
	auto title = windowTitle();
	if (title.endsWith('*')) {
		title.remove(title.length() - 1, 1);
		setWindowTitle(title);
	}
	document()->unmark_modified();
}

void ViewerMainWindow::open_file() {
	if (!ask_save())
		return;
	document()->unload_file();

	auto file =
		QFileDialog::getOpenFileName(this, tr("Open VTF"), QString(), "Valve Texture Format (*.vtf);;All files (*.*)");
	if (file.isEmpty())
		return;

	if (!document()->load_file(file.toUtf8().data())) {
		QMessageBox::warning(this, tr("Error"), tr("Could not open file!"));
	}
}

// Promps the user for save if dirty
// Returns true if you should continue processing whatever request you were before calling this
bool ViewerMainWindow::ask_save() {
	if (!document()->dirty())
		return true;

	auto msgBox = new QMessageBox(
		QMessageBox::Icon::Question, tr("Save changes?"), tr("You have unsaved changes. Would you like to save?"),
		QMessageBox::NoButton, this);
	msgBox->addButton(QMessageBox::Save);
	msgBox->addButton(QMessageBox::Cancel);
	msgBox->addButton(QMessageBox::Discard);

	auto r = msgBox->exec();

	if (r == QMessageBox::Cancel) {
		return false;
	}
	else if (r == QMessageBox::Save) {
		document()->save();
	}

	return true;
}

void ViewerMainWindow::save(bool saveAs) {
	if (!document()->dirty())
		return;

	// Ask for a save directory if there's no active file
	if (document()->path().empty() || saveAs) {
		auto name = QFileDialog::getSaveFileName(this, tr("Save as"), QString(), "Valve Texture File (*.vtf)");
		if (name.isEmpty())
			return;
		document()->set_path(name.toUtf8().data());
	}

	if (!document()->save()) {
		QMessageBox::warning(
			this, "Could not save file!", fmt::format(FMT_STRING("Failed to save file: {}"), vlGetLastError()).c_str(),
			QMessageBox::Ok);
		return;
	}

	unmark_modified();
}

void ViewerMainWindow::new_file() {
	if (!ask_save())
		return;

	document()->new_file();
}

void ViewerMainWindow::reload_file() {
	if (!ask_save())
		return;

	document()->reload_file();
}

void ViewerMainWindow::import_file() {
	auto filename = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(),"JPG Files (*.jpg)").toUtf8();

//	ImageImportDialog dialog(this);
//	dialog.exec();

	if (filename.isEmpty())
		return;

	auto image = imglib::image_begin(filename);

	auto data = imglib::image_load(image);

	auto file = new CVTFFile();

	// TODO: Properly handle different image formats
	file->Create(data.info.w, data.info.h, 1, 1, 1, VTFImageFormat::IMAGE_FORMAT_RGB888);
	file->SetData(1, 1, 1, 0, (vlByte*)data.data);

	document()->load_file(file);

	// free image data
	imglib::image_free(data);
	imglib::image_end(image);
}

//////////////////////////////////////////////////////////////////////////////////
// InfoWidget
//////////////////////////////////////////////////////////////////////////////////

static inline constexpr const char* INFO_FIELDS[] = {
	"Width", "Height", "Depth", "Frames", "Faces", "Mips", "Reflectivity"};

static inline constexpr const char* FILE_FIELDS[] = {"Size", "Version"};

static inline constexpr struct {
	VTFImageFormat format;
	const char* name;
} IMAGE_FORMATS[] = {
	{IMAGE_FORMAT_RGBA8888, "RGBA8888"},
	{IMAGE_FORMAT_ABGR8888, "ABGR8888"},
	{IMAGE_FORMAT_RGB888, "RGB888"},
	{IMAGE_FORMAT_BGR888, "BGR888"},
	{IMAGE_FORMAT_RGB565, "RGB565"},
	{IMAGE_FORMAT_I8, "I8"},
	{IMAGE_FORMAT_IA88, "IA88"},
	{IMAGE_FORMAT_P8, "P8"},
	{IMAGE_FORMAT_A8, "A8"},
	{IMAGE_FORMAT_RGB888_BLUESCREEN, "RGB888_BLUESCREEN"},
	{IMAGE_FORMAT_BGR888_BLUESCREEN, "BGR888_BLUESCREEN"},
	{IMAGE_FORMAT_ARGB8888, "ARGB8888"},
	{IMAGE_FORMAT_BGRA8888, "BGRA8888"},
	{IMAGE_FORMAT_DXT1, "DXT1"},
	{IMAGE_FORMAT_DXT3, "DXT3"},
	{IMAGE_FORMAT_DXT5, "DXT5"},
	{IMAGE_FORMAT_BGRX8888, "BGRX8888"},
	{IMAGE_FORMAT_BGR565, "BGR565"},
	{IMAGE_FORMAT_BGRX5551, "BGRX5551"},
	{IMAGE_FORMAT_BGRA4444, "BGRA4444"},
	{IMAGE_FORMAT_DXT1_ONEBITALPHA, "DXT1_ONEBITALPHA"},
	{IMAGE_FORMAT_BGRA5551, "BGRA5551"},
	{IMAGE_FORMAT_UV88, "UV88"},
	{IMAGE_FORMAT_UVWQ8888, "UVWQ8888"},
	{IMAGE_FORMAT_RGBA16161616F, "RGBA16161616F"},
	{IMAGE_FORMAT_RGBA16161616, "RGBA16161616"},
	{IMAGE_FORMAT_UVLX8888, "UVLX8888"},
	{IMAGE_FORMAT_R32F, "R32F"},
	{IMAGE_FORMAT_RGB323232F, "RGB323232F"},
	{IMAGE_FORMAT_RGBA32323232F, "RGBA32323232F"},
	{IMAGE_FORMAT_NV_DST16, "NV_DST16"},
	{IMAGE_FORMAT_NV_DST24, "NV_DST24"},
	{IMAGE_FORMAT_NV_INTZ, "NV_INTZ"},
	{IMAGE_FORMAT_NV_RAWZ, "NV_RAWZ"},
	{IMAGE_FORMAT_ATI_DST16, "ATI_DST16"},
	{IMAGE_FORMAT_ATI_DST24, "ATI_DST24"},
	{IMAGE_FORMAT_NV_NULL, "NV_NULL"},
	{IMAGE_FORMAT_ATI2N, "ATI2N"},
	{IMAGE_FORMAT_ATI1N, "ATI1N"},
};

InfoWidget::InfoWidget(QWidget* pParent)
	: QWidget(pParent) {
	setup_ui();
}

void InfoWidget::update_info(VTFLib::CVTFFile* file) {
	for (auto& pair : fields_) {
		pair.second->clear();
	}

	if (!file)
		return;

	find("Width")->setText(QString::number(file->GetWidth()));
	find("Height")->setText(QString::number(file->GetHeight()));
	find("Depth")->setText(QString::number(file->GetDepth()));
	find("Frames")->setText(QString::number(file->GetFrameCount()));
	find("Faces")->setText(QString::number(file->GetFaceCount()));
	find("Mips")->setText(QString::number(file->GetMipmapCount()));

	find("Version")->setText(QString::number(file->GetMajorVersion()) + "." + QString::number(file->GetMinorVersion()));
	auto size = file->GetSize();
	find("Size")->setText(
		fmt::format(FMT_STRING("{:.2f} MiB ({:.2f} KiB)"), size / (1024.f * 1024.f), size / 1024.f).c_str());

	vlSingle x, y, z;
	file->GetReflectivity(x, y, z);
	find("Reflectivity")->setText(fmt::format(FMT_STRING("{:.3f} {:.3f} {:.3f}"), x, y, z).c_str());

	// Select the correct image format
	for (int i = 0; i < util::ArraySize(IMAGE_FORMATS); ++i) {
		if (IMAGE_FORMATS[i].format == file->GetFormat()) {
			formatCombo_->setCurrentIndex(i);
			break;
		}
	}
}

void InfoWidget::setup_ui() {
	auto* layout = new QVBoxLayout(this);
	auto* fileGroupBox = new QGroupBox(tr("File Metadata"), this);
	auto* imageGroupBox = new QGroupBox(tr("Image Info"), this);

	auto* fileGroupLayout = new QGridLayout(fileGroupBox);
	auto* imageGroupLayout = new QGridLayout(imageGroupBox);
	fileGroupLayout->setColumnStretch(1, 1);
	imageGroupLayout->setColumnStretch(1, 1);

	// Prevent rows from expanding on resize
	fileGroupLayout->setRowStretch(util::ArraySize(FILE_FIELDS), 1);
	imageGroupLayout->setRowStretch(util::ArraySize(INFO_FIELDS), 1);

	// File meta info
	int row = 0;
	for (auto& f : FILE_FIELDS) {
		auto* label = new QLabel(QString(f) + ":", fileGroupBox);
		auto* edit = new QLineEdit(this);
		edit->setReadOnly(true);

		fileGroupLayout->addWidget(label, row, 0);
		fileGroupLayout->addWidget(edit, row, 1);
		++row;

		fields_.insert({f, edit});
	}

	// Image contents info group box below here
	row = 0;

	// Image format dropdown box
	formatCombo_ = new QComboBox(this);
	for (auto& fmt : IMAGE_FORMATS) {
		formatCombo_->addItem(fmt.name, (int)fmt.format);
	}
	imageGroupLayout->addWidget(new QLabel("Image format:", this), row, 0);
	imageGroupLayout->addWidget(formatCombo_, row, 1);
	++row;

	for (auto& f : INFO_FIELDS) {
		auto* label = new QLabel(QString(f) + ":", imageGroupBox);
		auto* edit = new QLineEdit(this);
		edit->setReadOnly(true);

		imageGroupLayout->addWidget(label, row, 0);
		imageGroupLayout->addWidget(edit, row, 1);
		++row;

		fields_.insert({f, edit});
	}

	layout->addWidget(fileGroupBox);
	layout->addWidget(imageGroupBox);

	// Prevent space being added to the bottom of the file metadata group box
	layout->addStretch(1);
}

//////////////////////////////////////////////////////////////////////////////////
// ImageViewWidget
//////////////////////////////////////////////////////////////////////////////////

ImageViewWidget::ImageViewWidget(QWidget* pParent)
	: QWidget(pParent) {
	setMinimumSize(256, 256);
}

void ImageViewWidget::set_pixmap(const QImage& pixmap) {
	image_ = pixmap;
}

void ImageViewWidget::set_vtf(VTFLib::CVTFFile* file) {
	file_ = file;
	// Force refresh of data
	currentFrame_ = -1;
	currentFace_ = -1;
	currentMip_ = -1;

	zoom_ = 1.f;
	pos_ = {0, 0};
	// No file, sad.
	if (!file)
		return;

	update_size();
}

void ImageViewWidget::paintEvent(QPaintEvent* event) {
	QPainter painter(this);

	if (!file_)
		return;

	// Compute draw size for this mip, frame, etc
	vlUInt imageWidth, imageHeight, imageDepth;
	CVTFFile::ComputeMipmapDimensions(
		file_->GetWidth(), file_->GetHeight(), file_->GetDepth(), mip_, imageWidth, imageHeight, imageDepth);
	// Needs decode
	if (frame_ != currentFrame_ || mip_ != currentMip_ || face_ != currentFace_) {
		const bool hasAlpha = CVTFFile::GetImageFormatInfo(file_->GetFormat()).uiAlphaBitsPerPixel > 0;
		const VTFImageFormat format = hasAlpha ? IMAGE_FORMAT_RGBA8888 : IMAGE_FORMAT_RGB888;
		auto size = CVTFFile::ComputeMipmapSize(file_->GetWidth(), file_->GetHeight(), 1, mip_, format);
		if (imgBuf_) {
			free(imgBuf_);
		}
		// This buffer needs to persist- QImage does not own the mem you give it
		imgBuf_ = static_cast<vlByte*>(malloc(size));

		bool ok = CVTFFile::Convert(
			file_->GetData(frame_, face_, 0, mip_), (vlByte*)imgBuf_, imageWidth, imageHeight, file_->GetFormat(),
			format);
		if (!ok) {
			std::cerr << "Could not convert image for display.\n";
			return;
		}

		image_ = QImage(
			(uchar*)imgBuf_, imageWidth, imageHeight, hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
		currentFace_ = face_;
		currentFrame_ = frame_;
		currentMip_ = mip_;
	}

	QPoint destpt =
		QPoint(width() / 2, height() / 2) - QPoint((imageWidth * zoom_) / 2, (imageHeight * zoom_) / 2) + pos_;
	QRect target = QRect(destpt.x(), destpt.y(), image_.width() * zoom_, image_.height() * zoom_);

	painter.drawImage(target, image_, QRect(0, 0, image_.width(), image_.height()));
}

void ImageViewWidget::zoom(float amount) {
	if (amount == 0)
		return; // Skip expensive repaint
	zoom_ += amount;
	if (zoom_ < 0.1f)
		zoom_ = 0.1f;
	update_size();
}

void ImageViewWidget::update_size() {
	if (!file_)
		return;

	// Resize widget to be the same size as the image
	QSize sz(file_->GetWidth() * zoom_, file_->GetHeight() * zoom_);
	resize(sz);
}

//////////////////////////////////////////////////////////////////////////////////
// ResourceWidget
//////////////////////////////////////////////////////////////////////////////////

ResourceWidget::ResourceWidget(QWidget* parent)
	: QWidget(parent) {
	setup_ui();
}

void ResourceWidget::set_vtf(VTFLib::CVTFFile* file) {
	table_->clear();
	if (!file)
		return;

	auto resources = file->GetResourceCount();
	table_->setRowCount(resources);
	for (vlUInt i = 0; i < resources; ++i) {
		auto type = file->GetResourceType(i);
		vlUInt size;
		auto data = file->GetResourceData(type, size);

		table_->setItem(i, 0, new QTableWidgetItem(GetResourceName(type)));

		auto typeItem = new QTableWidgetItem(fmt::format(FMT_STRING("0x{:X}"), type).c_str());
		table_->setItem(i, 1, typeItem);

		auto sizeItem =
			new QTableWidgetItem(fmt::format(FMT_STRING("{:d} bytes ({:.2f} KiB)"), size, size / 1024.f).c_str());
		table_->setItem(i, 2, sizeItem);
	}
}

void ResourceWidget::setup_ui() {
	auto* layout = new QVBoxLayout(this);

	table_ = new QTableWidget(this);
	table_->setSelectionBehavior(QAbstractItemView::SelectRows);
	table_->verticalHeader()->hide();
	table_->setColumnCount(3);
	table_->horizontalHeader()->setStretchLastSection(true);
	table_->setHorizontalHeaderItem(0, new QTableWidgetItem("Resource Name"));
	table_->setHorizontalHeaderItem(1, new QTableWidgetItem("Resource Type"));
	table_->setHorizontalHeaderItem(2, new QTableWidgetItem("Data Size"));

	layout->addWidget(table_);
}

//////////////////////////////////////////////////////////////////////////////////
// Texture flag list
//////////////////////////////////////////////////////////////////////////////////

constexpr struct TextureFlag {
	uint32_t flag;
	const char* name;
} TEXTURE_FLAGS[] = {
	{TEXTUREFLAGS_POINTSAMPLE, "Point Sample"},
	{TEXTUREFLAGS_TRILINEAR, "Trilinear"},
	{TEXTUREFLAGS_CLAMPS, "Clamp S"},
	{TEXTUREFLAGS_CLAMPT, "Clamp T"},
	{TEXTUREFLAGS_CLAMPU, "Clamp U"},
	{TEXTUREFLAGS_ANISOTROPIC, "Anisotropic"},
	{TEXTUREFLAGS_HINT_DXT5, "Hint DXT5"},
	{TEXTUREFLAGS_SRGB, "sRGB"},
	{TEXTUREFLAGS_DEPRECATED_NOCOMPRESS, "Nocompress (Deprecated)"},
	{TEXTUREFLAGS_NORMAL, "Normal"},
	{TEXTUREFLAGS_NOMIP, "No MIP"},
	{TEXTUREFLAGS_NOLOD, "No LOD"},
	{TEXTUREFLAGS_MINMIP, "Min Mip"},
	{TEXTUREFLAGS_PROCEDURAL, "Procedural"},
	{TEXTUREFLAGS_ONEBITALPHA, "One-bit Alpha"},
	{TEXTUREFLAGS_EIGHTBITALPHA, "Eight-bit Alpha"},
	{TEXTUREFLAGS_ENVMAP, "Envmap"},
	{TEXTUREFLAGS_RENDERTARGET, "Render Target"},
	{TEXTUREFLAGS_DEPTHRENDERTARGET, "Depth Render Target"},
	{TEXTUREFLAGS_NODEBUGOVERRIDE, "No Debug Override"},
	{TEXTUREFLAGS_SINGLECOPY, "Single Copy"},
	{TEXTUREFLAGS_DEPRECATED_ONEOVERMIPLEVELINALPHA, "One Over Mip Level Linear Alpha (Deprecated)"},
	{TEXTUREFLAGS_DEPRECATED_PREMULTCOLORBYONEOVERMIPLEVEL, "Pre-multiply Colors by One Over Mip Level (Deprecated)"},
	{TEXTUREFLAGS_DEPRECATED_NORMALTODUDV, "Normal To DuDv"},
	{TEXTUREFLAGS_DEPRECATED_ALPHATESTMIPGENERATION, "Alpha Test Mip Generation (Deprecated)"},
	{TEXTUREFLAGS_NODEPTHBUFFER, "No Depth Buffer"},
	{TEXTUREFLAGS_DEPRECATED_NICEFILTERED, "Nice Filtered (Deprecated)"},
	{TEXTUREFLAGS_VERTEXTEXTURE, "Vertex Texture"},
	{TEXTUREFLAGS_SSBUMP, "SSBump"},
	{TEXTUREFLAGS_DEPRECATED_UNFILTERABLE_OK, "Unfilterable OK (Deprecated)"},
	{TEXTUREFLAGS_BORDER, "Border"},
	{TEXTUREFLAGS_DEPRECATED_SPECVAR_RED, "Specvar Red (Deprecated)"},
	{TEXTUREFLAGS_DEPRECATED_SPECVAR_ALPHA, "Specvar Alpha (Deprecated)"},
};

//////////////////////////////////////////////////////////////////////////////////
// ImageSettingsWidget
//////////////////////////////////////////////////////////////////////////////////
ImageSettingsWidget::ImageSettingsWidget(ImageViewWidget* viewer, QWidget* parent)
	: QWidget(parent) {
	setup_ui(viewer);
}

void ImageSettingsWidget::setup_ui(ImageViewWidget* viewer) {
	auto* layout = new QGridLayout(this);

	int row = 0;
	frame_ = new QSpinBox(this);
	connect(
		frame_, &QSpinBox::textChanged,
		[viewer, this](const QString&)
		{
			viewer->set_frame(frame_->value());
		});
	layout->addWidget(frame_, row, 1);
	layout->addWidget(new QLabel("Frame:"), row, 0);

	++row;
	mip_ = new QSpinBox(this);
	connect(
		mip_, &QSpinBox::textChanged,
		[viewer, this](const QString&)
		{
			viewer->set_mip(mip_->value());
		});
	layout->addWidget(mip_, row, 1);
	layout->addWidget(new QLabel("Mip:"), row, 0);

	++row;
	face_ = new QSpinBox(this);
	connect(
		face_, &QSpinBox::textChanged,
		[viewer, this](const QString&)
		{
			viewer->set_face(face_->value());
		});
	layout->addWidget(face_, row, 1);
	layout->addWidget(new QLabel("Face:"), row, 0);

	++row;
	startFrame_ = new QSpinBox(this);
	connect(
		startFrame_, &QSpinBox::textChanged,
		[this](const QString&)
		{
			if (!file_)
				return;
			file_->SetStartFrame(startFrame_->value());
			if (!settingFile_)
				emit fileModified();
		});
	layout->addWidget(startFrame_, row, 1);
	layout->addWidget(new QLabel("Start Frame:"), row, 0);

	// Flags list box
	++row;
	auto* flagsScroll = new QScrollArea(this);
	auto* flagsGroup = new QGroupBox(tr("Flags"), this);
	auto* flagsLayout = new QGridLayout(flagsGroup);

	for (auto& flag : TEXTURE_FLAGS) {
		auto* check = new QCheckBox(flag.name, this);
		check->setCheckable(true);
		connect(
			check, &QCheckBox::stateChanged,
			[this, flag](int newState)
			{
				if (!file_)
					return;
				file_->SetFlag((VTFImageFlag)flag.flag, newState);
				if (!settingFile_)
					emit fileModified();
			});
		flagChecks_.insert({flag.flag, check});
		flagsLayout->addWidget(check);
	}

	flagsScroll->setWidget(flagsGroup);
	layout->addWidget(flagsScroll, row, 0, 1, 2);
}

void ImageSettingsWidget::set_vtf(VTFLib::CVTFFile* file) {
	// Hack to ensure we don't emit fileModified when setting defaults
	settingFile_ = true;

	// Set some sensible defaults in the event no file is loaded
	if (!file) {
		startFrame_->setValue(0);
		startFrame_->setRange(0, 0);
		mip_->setValue(0);
		mip_->setRange(0, 0);
		face_->setValue(0);
		face_->setRange(0, 0);
		frame_->setValue(0);
		frame_->setRange(0, 0);
		for (auto& check : flagChecks_) {
			check.second->setChecked(false);
			check.second->setCheckable(false);
		}
		settingFile_ = false;
		return;
	}

	file_ = file;
	startFrame_->setValue(file->GetStartFrame());
	mip_->setValue(0);
	frame_->setValue(file->GetStartFrame());
	face_->setValue(0);

	// Configure ranges
	mip_->setRange(0, file->GetMipmapCount());
	frame_->setRange(1, file->GetFrameCount());
	face_->setRange(1, file->GetFaceCount());
	startFrame_->setRange(1, file->GetFrameCount());

	// Set the flags
	uint32_t flags = file->GetFlags();
	for (auto& f : TEXTURE_FLAGS) {
		auto check = flagChecks_.find(f.flag)->second;
		check->setChecked(!!(flags & f.flag));
		check->setCheckable(true);
	}

	settingFile_ = false;
}