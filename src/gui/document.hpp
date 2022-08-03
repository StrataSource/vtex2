
#pragma once

#include <QObject>

#include "VTFLib.h"

namespace vtfview
{

	/**
	 * Represents a VTF file document
	 * Handles saving, loading, etc
	 */
	class Document : public QObject {
		Q_OBJECT;

	public:
		explicit Document(QObject* parent);

		bool load_file(const char* path);
		bool load_file(const void* data, size_t size);
		bool load_file(VTFLib::CVTFFile* file);
		void unload_file();

		inline auto* file() {
			return file_;
		}
		inline const auto* file() const {
			return file_;
		}

		void mark_modified();
		void unmark_modified();

		bool save(const std::string& path = "");
		void new_file();
		void reload_file();

		const std::string& path() const {
			return path_;
		}
		void set_path(const std::string& p) {
			path_ = p;
		}

		bool dirty() const {
			return dirty_;
		}

		void set_format(VTFImageFormat format);

	signals:
		/**
		 * Invoked whenever the vtf changes
		 * file may be nullptr if the file is fully unloaded
		 * path may be empty if this file is not loaded from disk
		 */
		void vtfFileChanged(const std::string& path, VTFLib::CVTFFile* file);

		/**
		 * Invoked when the modification state of the VTF changed
		 * @param modified True if file is modified, false if not
		 */
		void vtfFileModified(bool modified);

	protected:
		bool load_file_internal(const void* bytes, size_t size);

	private:
		VTFLib::CVTFFile* file_ = nullptr;
		bool dirty_ = false;
		std::string path_;
		VTFImageFormat format_ = IMAGE_FORMAT_NONE;
	};

} // namespace vtfview
