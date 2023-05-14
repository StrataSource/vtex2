
#include <QFileDialog>
#include <QMessageBox>

#include <fmt/format.h>
#include "nameof.hpp"

#include "document.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"

using namespace vtfview;
using namespace VTFLib;

Document::Document(QObject* parent)
	: QObject(parent) {
	history_.push_back({});
}

void Document::new_file() {
	unload_file();
}

void Document::reload_file() {
	auto oldPath = path_;
	unload_file();
	load_file(oldPath.c_str());
	unmark_modified();
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
	if (data().format_ != IMAGE_FORMAT_NONE) {
		fmt::print(
			"Converting image from {} to {} on save...\n", NAMEOF_ENUM(file_->GetFormat()),
			NAMEOF_ENUM(data().format_));
		if (!file_->ConvertInPlace(data().format_))
			return false;
	}
	
	bool needsMipRegen = data().mips_ != file_->GetMipmapCount();
	
	// Image needs resize
	if (data().width_ != file_->GetWidth() || data().height_ != file_->GetHeight()) {
		needsMipRegen = true; // Always regenerate mips if we change size
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
	file_ = file;
	emit vtfFileChanged("", file);
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
	data().format_ = IMAGE_FORMAT_NONE;
}

bool Document::load_file_internal(const void* imgdata, size_t size) {
	auto* oldFile = file_;
	file_ = new VTFLib::CVTFFile();
	if (!file_->Load(imgdata, size)) {
		delete file_;
		file_ = oldFile;
		return false;
	}
	
	clear_history();
	data().height_ = file_->GetHeight();
	data().width_ = file_->GetWidth();
	data().mips_ = file_->GetMipmapCount();
	data().flags_ = file_->GetFlags();
	data().startFrame_ = file_->GetStartFrame();

	delete oldFile;
	return true;
}

void Document::set_format(VTFImageFormat format) {
	if (format == file_->GetFormat()) {
		data().format_ = IMAGE_FORMAT_NONE;
		return;
	}
	data().format_ = format;
	mark_modified();
}

void Document::set_height(int height) {
	if (height != data().height_) {
		data().height_ = height;
		mark_modified();
	}
}

void Document::set_width(int width) {
	if (data().width_ != width) {
		data().width_ = width;
		mark_modified();
	}
}

void Document::set_mips(int mipCount) {
	if (mipCount != data().mips_) {
		data().mips_ = mipCount;
		mark_modified();
	}
}

int Document::max_mips() const {
	return CVTFFile::ComputeMipmapCount(data().width_, data().height_, 1);
}

void Document::set_flag(VTFImageFlag flag, bool on) {
	if ((data().flags_ & flag) != (on ? flag : 0)) {
		if (on)
			data().flags_ |= flag;
		else
			data().flags_ &= ~flag;
		mark_modified();
	}
}

void Document::mark_history(const std::string& name) {
	// Erase any additional entries in the history (i.e. leftover from undo-ing)
	if (histIndex_ != history_.size()-1)
		history_.erase(history_.begin() + histIndex_ + 1, history_.end());
	history_.push_back({
		.name = name,
		.d = data()
	});
	histIndex_++;
}

bool Document::undo() {
	if (histIndex_ == 0)
		return false;
	histIndex_--;
	return true;
}

bool Document::redo() {
	if (histIndex_ == history_.size()-1)
		return false;
	histIndex_++;
	return true;
}

void Document::set_start_frame(int frame) {
	if (frame != data().startFrame_) {
		data().startFrame_ = frame;
		mark_modified();
	}
}

void Document::clear_history() {
	history_.clear();
	history_.push_back({});
	histIndex_ = 0;
}
