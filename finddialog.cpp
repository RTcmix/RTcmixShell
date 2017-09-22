#include <QtWidgets>
#include "finddialog.h"
#include "utils.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    okCancelButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    CHECKED_CONNECT(okCancelButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    CHECKED_CONNECT(okCancelButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setWindowTitle(tr("Find"));
}
