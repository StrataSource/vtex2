
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

#pragma once

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

		void save(bool saveAs = false);
		void open_file();
		void new_file();
		void reload_file();
		void import_file();

		bool load_file(const char* path) {
			return document()->load_file(path);
		}

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

		inline const QImage& pixmap() const {
			return image_;
		};
		inline QImage& pixmap() {
			return image_;
		};

		void paintEvent(QPaintEvent* event) override;

		void set_frame(int f) {
			frame_ = f;
			repaint();
		}
		void set_face(int f) {
			face_ = f;
			repaint();
		}
		void set_mip(int f) {
			mip_ = f;
			repaint();
		}

		void setZoom(float amount);
		inline void zoomIn(float amount = 0.1f) {
			setZoom(zoom_ + amount);
		}

		inline void zoomOut(float amount = 0.1f) {
			setZoom(zoom_ - amount);
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
