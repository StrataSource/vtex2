
#pragma once

#include <stack>
#include <QObject>

#include "VTFLib.h"

namespace vtfview
{

	/**
	 * Represents a VTF file document
	 * Handles saving, loading and modification of image parameters
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
		
		void set_width(int width);
		void set_height(int height);
		void set_mips(int mipCount);
		
		void set_flag(VTFImageFlag flag, bool on);
		
		/**
		 * Get or set the starting frame
		 */
		int start_frame() const { return data().startFrame_; }
		
		void set_start_frame(int frame);
		
		/**
		 * Returns currently set flags
		 */
		uint32_t flags() const { return data().flags_; }
		
		/**
		 * Returns the size this image will be resized to on save
		 * For actual current data size, query size from file()
		 */
		inline int resize_width() const { return data().width_; }
		inline int resize_height() const { return data().height_; }
		
		/**
		 * Returns how many mip levels we can have for this image, based on current
		 * width and height
		 */
		int max_mips() const;

		/**
		 * Marks a point in history, allowing an undo/redo operation
		 * Any operations on the stack that exist above this will be erased
		 * @param name Name of the history point, for display in UI
		 */
		void mark_history(const std::string& name);
		
		/**
		 * Undo the last operation
		 * @returns True if we undid something
		 */
		bool undo();
		
		/**
		 * Redo any previously undone operations.
		 * Traverses upwards in the history stack
		 * @returns True if we redid something
		 */
		bool redo();

		/**
		 * Clears all history and set it to default state
		 */
		void clear_history();

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
		
		struct data_t {
			VTFImageFormat format_ = IMAGE_FORMAT_NONE; // Format to convert to on save, if none, don't convert
			int width_ = 0;
			int height_ = 0;
			int mips_ = 0;
			uint32_t flags_ = 0;
			int startFrame_;
		};
		
		struct history_point_t {
			std::string name;
			data_t d;
		};
		
		int histIndex_ = 0;
		std::vector<history_point_t> history_;
		
		inline data_t& data() {
			return history_[histIndex_].d;
		}
		
		inline const data_t& data() const {
			return history_[histIndex_].d;
		}
	};

} // namespace vtfview
