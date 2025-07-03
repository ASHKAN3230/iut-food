#ifndef RATE_DIALOG_H
#define RATE_DIALOG_H

#include <QDialog>

class QSpinBox;
class QTextEdit;
class QLabel;
class QPushButton;

class RateDialog : public QDialog {
    Q_OBJECT
public:
    RateDialog(const QString &restaurantName, QWidget *parent = nullptr);
    int rating() const;
    QString comment() const;
private:
    QSpinBox *ratingSpin;
    QTextEdit *commentEdit;
    QLabel *label;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // RATE_DIALOG_H 