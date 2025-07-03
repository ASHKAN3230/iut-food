#include "rate_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>

RateDialog::RateDialog(const QString &restaurantName, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Rate Order");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    label = new QLabel("Rate your order at " + restaurantName + ":", this);
    mainLayout->addWidget(label);
    QHBoxLayout *rateLayout = new QHBoxLayout();
    QLabel *rateLabel = new QLabel("Rating (1-5):", this);
    ratingSpin = new QSpinBox(this);
    ratingSpin->setRange(1, 5);
    ratingSpin->setValue(5);
    rateLayout->addWidget(rateLabel);
    rateLayout->addWidget(ratingSpin);
    mainLayout->addLayout(rateLayout);
    QLabel *commentLabel = new QLabel("Comment:", this);
    mainLayout->addWidget(commentLabel);
    commentEdit = new QTextEdit(this);
    commentEdit->setPlaceholderText("Leave a comment (optional)");
    mainLayout->addWidget(commentEdit);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    setLayout(mainLayout);
}

int RateDialog::rating() const {
    return ratingSpin->value();
}

QString RateDialog::comment() const {
    return commentEdit->toPlainText();
} 