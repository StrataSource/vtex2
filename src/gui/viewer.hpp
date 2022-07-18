
#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>

#include <unordered_map>
#include <string>

#include "VTFLib.h"

#pragma once

class QSpinBox;
class QCheckBox;

namespace vtfview {
	
	/**
	 * Main window container for the VTF viewer
	 */
	class ViewerMainWindow : public QMainWindow {
		Q_OBJECT;
		
	public:
		ViewerMainWindow(QWidget* pParent = nullptr);
		
		bool load_file(const char* path);
		bool load_file(const void* data, size_t size);
		bool load_file(VTFLib::CVTFFile* file);
		void unload_file();
		
		inline auto* file() { return file_; }
		inline const auto* file() const { return file_; }
		
		void mark_modified();
		
	protected:
		void setup_ui();
		void reset_state();
		
	signals:
		/**
		 * Invoked whenever the vtf changes
		 * file may be nullptr if the file is fully unloaded
		 */
		void vtfFileChanged(VTFLib::CVTFFile* file);
		
	private:
		VTFLib::CVTFFile* file_;
		bool dirty_;
	};
	
	/**
	 * General VTF info widget
	 */
	class InfoWidget : public QWidget {
		Q_OBJECT;
	public:
		InfoWidget(QWidget* pParent = nullptr);
		
		/**
		 * Update the widget with info from the specified VTF file
		 */
		void update_info(VTFLib::CVTFFile* file);
		
	private:
		void setup_ui();
		inline QLineEdit* find(const std::string& l) { return fields_.find(l)->second; }
		
		std::unordered_map<std::string, QLineEdit*> fields_;
	};
	
	/**
	 * Simple image viewer widget
	 */
	class ImageViewWidget : public QWidget {
		Q_OBJECT;
	public:
		ImageViewWidget(QWidget* pParent = nullptr);
		
		void set_pixmap(const QImage& pixmap);
		void set_vtf(VTFLib::CVTFFile* file);
		
		inline const QImage& pixmap() const { return image_; };
		inline QImage& pixmap() { return image_; };
		
		void paintEvent(QPaintEvent* event) override;
		
		void set_frame(int f) { frame_ = f; repaint(); }
		void set_face(int f) { face_ = f; repaint(); }
		void set_mip(int f) { mip_ = f; repaint(); }
		
	private:
		QImage image_;
		void* imgBuf_ = nullptr;
		VTFLib::CVTFFile* file_;
		
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
		ResourceWidget(QWidget* parent = nullptr);
		
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
		ImageSettingsWidget(ImageViewWidget* viewer, QWidget* parent = nullptr);
		
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
	};
	
}
