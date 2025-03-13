#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>

#include <unordered_map>
#include <string>
#include <cassert>
#include <vector>

#include "VTFLib.h"

#include "document.hpp"
#include "common/util.hpp"

class QSpinBox;
class QCheckBox;
class QComboBox;
class QShortcut;

namespace vtfview
{
	class ImageViewWidget;

	/**
	 * Main window container for the VTF viewer
	 */
	class ViewerMainWindow : public QMainWindow {
		Q_OBJECT;

	public:
		ViewerMainWindow(QWidget* pParent = nullptr);

		inline auto* document() {
			return doc_;
		}
		inline const auto* document() const {
			return doc_;
		}

	public slots:
		void on_save(bool saveAs = false);
		void on_open_file();
		void on_new_file();
		void on_reload_file();
		void on_import_file();
		void on_export_file();

	public:
		bool load_file(const char* path) {
			return document()->load_file(path);
		}

		bool import_file(const char* path);
		bool export_file(const char* path);

		enum Actions {
			Save,
			SaveAs,
			Load,
			Reload,
			ZoomIn,
			ZoomOut,

			Action_Count,
		};

		QShortcut* get_shortcut(Actions act) {
			assert(act < Action_Count && act >= 0);
			return shortcuts_[act];
		}

	protected:
		void setup_ui();
		void setup_menubar();
		void reset_state();

		void closeEvent(QCloseEvent* event) override;

		void mark_modified();
		void unmark_modified();

		bool ask_save();

	private:
		ImageViewWidget* viewer_ = nullptr;
		Document* doc_ = nullptr;

		std::vector<QShortcut*> shortcuts_;
	};

	/**
	 * General VTF info widget
	 */
	class InfoWidget : public QWidget {
		Q_OBJECT;

	public:
		InfoWidget(Document* doc, QWidget* pParent = nullptr);

		/**
		 * Update the widget with info from the specified VTF file
		 */
		void update_info(VTFLib::CVTFFile* file);

	private:
		void setup_ui();
		inline QLineEdit* find(const std::string& l) {
			return fields_.find(l)->second;
		}

		std::unordered_map<std::string, QLineEdit*> fields_;
		QComboBox* formatCombo_ = nullptr;
		QComboBox* versionCombo_ = nullptr;
		Document* doc_ = nullptr;
	};

	/**
	 * Simple image viewer widget
	 */
	class ImageViewWidget : public QWidget {
		Q_OBJECT;

	public:
		ImageViewWidget(Document* doc, QWidget* pParent = nullptr);

		void set_pixmap(const QImage& pixmap);
		void set_vtf(VTFLib::CVTFFile* file);

		// Should probably make the checkerboard size dependent on viewer size...
		static constexpr inline int checkerboard_size = 512;
		//const int checkerboard_quality = 16;
		static constexpr inline int checkerboard_divisor = 16;
		QImage checkerboard;

		inline const QImage& pixmap() const {
			return image_;
		};
		inline QImage& pixmap() {
			return image_;
		};

		void paintEvent(QPaintEvent* event) override;

		void set_frame(int f) {
			frame_ = file_ ? util::clamp(f, 1, file_->GetFrameCount()) : 1;
			frame_--;
			repaint();
		}

		void set_face(int f) {
			face_ = file_ ? util::clamp(f, 1, file_->GetFaceCount()) : 1;
			face_--;
			repaint();
		}

		// Sets the current mip
		// This is in the range 1-mipcount, in vtflib it's 0-based index
		void set_mip(int f) {
			mip_ = file_ ? util::clamp(f, 1, file_->GetMipmapCount()) : 1;
			mip_--;
			repaint();
		}

		int get_mip() {
			return mip_;
		}

		void zoom(float amount);
		inline void zoomIn(float amount = 0.1f) {
			zoom(amount);
		}

		inline void zoomOut(float amount = 0.1f) {
			zoom(amount * -1);
		}

	private:
		void update_size();

		QImage image_;
		void* imgBuf_ = nullptr;
		VTFLib::CVTFFile* file_ = nullptr;

		float zoom_ = 1.0f;
		QPoint pos_;

		int frame_ = 0;
		int face_ = 0;
		int mip_ = 0;

		int currentFrame_ = 0;
		int currentFace_ = 0;
		int currentMip_ = 0;
	};

	/**
	 * Resource list widget
	 */
	class ResourceWidget : public QWidget {
		Q_OBJECT;

	public:
		ResourceWidget(Document* doc, QWidget* parent = nullptr);

		void set_vtf(VTFLib::CVTFFile* file);

	private:
		void setup_ui();

		QTableWidget* table_;
	};

	/**
	 * Viewer settings
	 * Lets you select mip, frame, face, etc.
	 */
	class ImageSettingsWidget : public QWidget {
		Q_OBJECT;

	public:
		ImageSettingsWidget(Document* doc, ImageViewWidget* viewer, QWidget* parent = nullptr);

		void set_vtf(VTFLib::CVTFFile* file);

	signals:
		/**
		 * Invoked when the VTF is modified in some way
		 * ie by start frame being changed
		 */
		void fileModified();

	private:
		void setup_ui(ImageViewWidget* viewer);

		QSpinBox* frame_ = nullptr;
		QSpinBox* face_ = nullptr;
		QSpinBox* mip_ = nullptr;
		QSpinBox* startFrame_ = nullptr;
		VTFLib::CVTFFile* file_ = nullptr;
		std::unordered_map<uint32_t, QCheckBox*> flagChecks_;
		bool settingFile_ = false;
	};

} // namespace vtfview
