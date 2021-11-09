from PySide6.QtWidgets import QApplication
import sys
from editor import SafeObjectEditor

if __name__ == "__main__":
    app = QApplication()
    editor = SafeObjectEditor()
    editor.show()
    
    sys.exit(app.exec())