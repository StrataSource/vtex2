
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QDockWidget>
#include <QPainter>
#include <QHeaderView>

#include "fmt/format.h"

#include "viewer.hpp"

#include "common/util.hpp"
#include "common/enums.hpp"

using namespace vtfview;
using namespace VTFLib;

ViewerMainWindow::ViewerMainWindow(QWidget* pParent) :
	QMainWindow(pParent) {
	setup_ui();
}


bool ViewerMainWindow::load_file(const char* path) {
	std::uint8_t* buffer;
	auto numRead = util::read_file(path, buffer);
	if (!numRead)
		return false;
		
	bool ok = load_file(buffer, numRead);
	delete buffer;
	return ok;
}

bool ViewerMainWindow::load_file(const void* data, size_t size) {
	file_ = new VTFLib::CVTFFile();
	if (!file_->Load(data, size)) {
		delete file_;
		file_ = nullptr;
		return false;
	}
	return load_file(file_);
}

bool ViewerMainWindow::load_file(VTFLib::CVTFFile* file) {
	emit vtfFileChanged(file);
	file_ = file;
	return true;
}

void ViewerMainWindow::unload_file() {
	if (!file_)
		return;
	emit vtfFileChanged(nullptr);
	delete file_;
	file_ = nullptr;
}

void ViewerMainWindow::setup_ui() {
	
	setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);
	
	// Info widget
	auto* infoDock = new QDockWidget(tr("Info"), this);
	
	auto* infoWidget = new InfoWidget(this);
	connect(this, &ViewerMainWindow::vtfFileChanged, infoWidget, &InfoWidget::update_info);

	infoDock->setWidget(infoWidget);
	addDockWidget(Qt::LeftDockWidgetArea, infoDock);
	
	// Resource list
	auto* resDock = new QDockWidget(tr("Resources"), this);
	
	auto* resList = new ResourceWidget(this);
	connect(this, &ViewerMainWindow::vtfFileChanged, resList, &ResourceWidget::set_vtf);
	
	resDock->setWidget(resList);
	addDockWidget(Qt::LeftDockWidgetArea, resDock);
	
	tabifyDockWidget(infoDock, resDock);
	
	// Main image viewer 
	auto* imageView = new ImageViewWidget(this);
	
	connect(this, &ViewerMainWindow::vtfFileChanged, [imageView](VTFLib::CVTFFile* file) {
		imageView->set_vtf(file);
	});
	setCentralWidget(imageView);
}

void ViewerMainWindow::reset_state() {
	dirty_ = false;
}

//////////////////////////////////////////////////////////////////////////////////
// InfoWidget
//////////////////////////////////////////////////////////////////////////////////

static inline constexpr const char* INFO_FIELDS[] = {
	"Width", "Height", "Depth",
	"Frames", "Faces", "Mips",
	"Image format",
	"Reflectivity"
};

static inline constexpr const char* FILE_FIELDS[] = {
	"Size", "Version"
};

InfoWidget::InfoWidget(QWidget* pParent) :
	QWidget(pParent) {
	setup_ui();
}

void InfoWidget::update_info(VTFLib::CVTFFile* file) {
	find("Width")->setText(QString::number(file->GetWidth()));
	find("Height")->setText(QString::number(file->GetHeight()));
	find("Depth")->setText(QString::number(file->GetDepth()));
	find("Frames")->setText(QString::number(file->GetFrameCount()));
	find("Faces")->setText(QString::number(file->GetFaceCount()));
	find("Mips")->setText(QString::number(file->GetMipmapCount()));
	find("Image format")->setText(ImageFormatToString(file->GetFormat()));
	
	find("Version")->setText(QString::number(file->GetMajorVersion()) + "." + QString::number(file->GetMinorVersion()));
	auto size = file->GetSize();
	find("Size")->setText(
		fmt::format(FMT_STRING("{:.2f} MiB ({:.2f} KiB)"), size / (1024.f*1024.f), size / 1024.f).c_str()
	);
	
	vlSingle x, y, z;
	file->GetReflectivity(x, y, z);
	find("Reflectivity")->setText(
		fmt::format(FMT_STRING("{:.3f} {:.3f} {:.3f}"), x, y, z).c_str()
	);
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
	
	// Image contents info
	row = 0;
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

ImageViewWidget::ImageViewWidget(QWidget* pParent) :
	QWidget(pParent) {
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
	pos_ = {0,0};
}

void ImageViewWidget::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	
	// Compute draw size for this mip, frame, etc
	vlUInt imageWidth, imageHeight, imageDepth;
	CVTFFile::ComputeMipmapDimensions(file_->GetWidth(), file_->GetHeight(), file_->GetDepth(), mip_, imageWidth, imageHeight, imageDepth);
		
	// Needs decode
	if (frame_ != currentFrame_ || mip_ != currentMip_ || face_ != currentFace_) {
		
		const bool hasAlpha = CVTFFile::GetImageFormatInfo(file_->GetFormat()).uiAlphaBitsPerPixel > 0;
		const VTFImageFormat format = hasAlpha ? IMAGE_FORMAT_RGBA8888 : IMAGE_FORMAT_RGB888;
		auto size = file_->ComputeMipmapSize(file_->GetWidth(), file_->GetHeight(), 1, mip_, format);
		
		auto* data = static_cast<vlByte*>(malloc(size));
		bool ok = CVTFFile::Convert(file_->GetData(frame_, face_, 0, mip_), data, imageWidth, imageHeight, file_->GetFormat(), format);
			
		if (!ok) {
			fprintf(stderr, "Could not convert image for display.\n");
			return;
		}
		
		image_ = QImage(data, imageWidth, imageHeight, hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
		
		currentFace_ = face_;
		currentFrame_ = frame_;
		currentMip_ = mip_;
	}
	
	QPoint center = QPoint(width()/2, height()/2) - QPoint(imageWidth/2, imageHeight/2);
	painter.drawImage(center + pos_, image_);
}

//////////////////////////////////////////////////////////////////////////////////
// ResourceWidget
//////////////////////////////////////////////////////////////////////////////////

ResourceWidget::ResourceWidget(QWidget* parent) :
	QWidget(parent) {
	setup_ui();
}

void ResourceWidget::set_vtf(VTFLib::CVTFFile* file) {
	table_->clear();
	
	auto resources = file->GetResourceCount();
	table_->setRowCount(resources);
	for (vlUInt i = 0; i < resources; ++i) {
		auto type = file->GetResourceType(i);
		vlUInt size;
		auto data = file->GetResourceData(type, size);
		
		table_->setItem(i, 0, new QTableWidgetItem(GetResourceName(type)));
		
		auto typeItem = new QTableWidgetItem(
			fmt::format(FMT_STRING("0x{:X}"), type).c_str()
		);
		table_->setItem(i, 1, typeItem);
		
		auto sizeItem = new QTableWidgetItem(
			fmt::format(FMT_STRING("{:d} bytes ({:.2f} KiB)"), size, size / 1024.f).c_str()
		);
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
