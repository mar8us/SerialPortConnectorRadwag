#include "message_utils.h"
#include <QMessageBox>

namespace Messages
{

bool showYesNoWarning(QWidget *parent, const QString &message)
{
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Ostrzeżenie");
    msgBox.setText(message);
    QPushButton *yesButton = msgBox.addButton("Tak", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("Nie", QMessageBox::NoRole);
    msgBox.setDefaultButton(yesButton);
    msgBox.exec();

    return msgBox.buttonRole(msgBox.clickedButton()) == QMessageBox::YesRole;
}

} //end_namespace Messages
