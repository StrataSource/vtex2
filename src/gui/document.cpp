
#include <QFileDialog>
#include <QMessageBox>

#include <fmt/format.h>

#include "document.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"

using namespace vtfview;

Document::Document(QObject* parent)
	: QObject(parent) {
}

void Document::new_file() {
	unload_file();
}

void Document::reload_file() {
	auto oldPath = path_;
	unload_file();
	load_file(oldPath.c_str());
}

void Document::mark_modified() {
	dirty_ = true;
	emit vtfFileModified(true);
}

void Document::unmark_modified() {
	dirty_ = false;
	emit vtfFileModified(false);
}

bool Document::save(const std::string& path) {
	if (!dirty_)
		return true;

	// Image needs converting
	if (format_ != IMAGE_FORMAT_NONE) {
		fmt::print(
			"Converting image from {} to {} on save...\n", ImageFormatToString(file_->GetFormat()),
			ImageFormatToString(format_));
		if (!file_->ConvertInPlace(format_))
			return false;
	}

	if (!file_->Save(path.empty() ? path_.c_str() : path.c_str())) {
		return false;
	}

	unmark_modified();
	return true;
}

bool Document::load_file(const char* path) {
	std::uint8_t* buffer;
	auto numRead = util::read_file(path, buffer);
	if (!numRead)
		return false;

	bool ok = load_file_internal(buffer, numRead);
	delete[] buffer;

	path_ = path;
	emit vtfFileChanged(path_, file_);
	return ok;
}

bool Document::load_file(const void* data, size_t size) {
	if (!load_file_internal(data, size))
		return false;
	emit vtfFileChanged("", file_);
	return true;
}

bool Document::load_file(VTFLib::CVTFFile* file) {
	emit vtfFileChanged("", file);
	file_ = file;
	path_ = "";
	return true;
}

void Document::unload_file() {
	if (!file_)
		return;
	emit vtfFileChanged("", nullptr);
	delete file_;
	file_ = nullptr;
	path_ = "";
	unmark_modified();
	format_ = IMAGE_FORMAT_NONE;
}

bool Document::load_file_internal(const void* data, size_t size) {
	auto* oldFile = file_;
	file_ = new VTFLib::CVTFFile();
	if (!file_->Load(data, size)) {
		delete file_;
		file_ = oldFile;
		return false;
	}
	delete oldFile;
	return true;
}

void Document::set_format(VTFImageFormat format) {
	if (format == file_->GetFormat()) {
		format_ = IMAGE_FORMAT_NONE;
		return;
	}
	format_ = format;
	mark_modified();
}
