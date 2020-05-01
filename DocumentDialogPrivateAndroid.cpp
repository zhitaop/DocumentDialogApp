#include "DocumentDialogPrivateAndroid.h"
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QDebug>
#include "JniEnvironment.h"

#define DOCUMENT_DIALOG_REQUEST_CODE 12

DocumentDialogPrivateAndroid::DocumentDialogPrivateAndroid(QObject* parent) :
    DocumentDialogPrivate(parent)
{
}

void DocumentDialogPrivateAndroid::open()
{
    JniEnvironment env;

    QAndroidJniObject ACTION_OPEN_DOCUMENT = QAndroidJniObject::getStaticObjectField("android/content/Intent", "ACTION_OPEN_DOCUMENT", "Ljava/lang/String;");
    qCInfo(documentDialog, "ACTION_OPEN_DOCUMENT = %s", ACTION_OPEN_DOCUMENT.toString().toUtf8().constData());

    QAndroidJniObject CATEGORY_OPENABLE = QAndroidJniObject::getStaticObjectField("android/content/Intent", "CATEGORY_OPENABLE", "Ljava/lang/String;");
    qCInfo(documentDialog, "CATEGORY_OPENABLE = %s", CATEGORY_OPENABLE.toString().toUtf8().constData());

    QAndroidJniObject mimeType = QAndroidJniObject::fromString(QStringLiteral( "*/*"));

    QAndroidJniObject intent = QAndroidJniObject("android/content/Intent");
    intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", ACTION_OPEN_DOCUMENT.object<jstring>());

    intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", ACTION_OPEN_DOCUMENT.object<jstring>());
    intent.callObjectMethod("addCategory", "(Ljava/lang/String;)Landroid/content/Intent;", CATEGORY_OPENABLE.object<jstring>());
    intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", mimeType.object<jstring>());
    QtAndroid::startActivity(intent.object<jobject>(), DOCUMENT_DIALOG_REQUEST_CODE, this);
}

void DocumentDialogPrivateAndroid::handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data)
{
    JniEnvironment env;

    qCInfo(documentDialog, "receiverRequestCode: %d resultCode: %d", receiverRequestCode, resultCode);

    jint RESULT_OK = QAndroidJniObject::getStaticField<jint>("android/app/Activity", "RESULT_OK");
    //jint RESULT_CANCELED = QAndroidJniObject::getStaticField<jint>("android/app/Activity", "RESULT_CANCELED");

    jint FLAG_GRANT_READ_URI_PERMISSION = QAndroidJniObject::getStaticField<jint>("android.content.Intent", "FLAG_GRANT_READ_URI_PERMISSION");
    qCInfo(documentDialog, "FLAG_GRANT_READ_URI_PERMISSION = %d", FLAG_GRANT_READ_URI_PERMISSION);

    jint FLAG_GRANT_WRITE_URI_PERMISSION = QAndroidJniObject::getStaticField<jint>("android.content.Intent", "FLAG_GRANT_WRITE_URI_PERMISSION");
    qCInfo(documentDialog, "FLAG_GRANT_WRITE_URI_PERMISSION = %d", FLAG_GRANT_WRITE_URI_PERMISSION);

    jint FLAG_GRANT_PERSISTABLE_URI_PERMISSION = QAndroidJniObject::getStaticField<jint>("android.content.Intent", "FLAG_GRANT_PERSISTABLE_URI_PERMISSION");
    qCInfo(documentDialog, "FLAG_GRANT_PERSISTABLE_URI_PERMISSION = %d", FLAG_GRANT_PERSISTABLE_URI_PERMISSION);

    if (receiverRequestCode != DOCUMENT_DIALOG_REQUEST_CODE)
    {
        emit rejected();
        return;
    }

    if (resultCode != RESULT_OK)
    {
        emit rejected();
        return;
    }

    QAndroidJniObject uri = data.callObjectMethod("getData", "()Landroid/net/Uri;" );
    qCInfo(documentDialog, "uri = %s", uri.toString().toUtf8().constData());

    QUrl fileUrl = uri.toString();
    qCInfo(documentDialog, "fileUrl = %s", fileUrl.toString().toUtf8().constData());

    jint flags = data.callMethod<jint>("getFlags", "()I");
    qCInfo(documentDialog, "flags (before) = %d", flags);

    flags &= (FLAG_GRANT_READ_URI_PERMISSION | FLAG_GRANT_WRITE_URI_PERMISSION);
    //qDebug() << Q_FUNC_INFO << "flags: " << flags;
    qCInfo(documentDialog, "flags (after) = %d", flags);

    QAndroidJniObject contentResolver = QtAndroid::androidActivity().callObjectMethod("getContentResolver","()Landroid/content/ContentResolver;");
    contentResolver.callMethod<void>("takePersistableUriPermission","(Landroid/net/Uri;I)V", uri.object<jobject>(), flags);
    qCInfo(documentDialog, "contentResolver.isValid = %d", contentResolver.isValid());

    /*
    QAndroidJniObject inputStream = contentResolver.callObjectMethod("openInputStream", "(Landroid/net/Uri;)Ljava/io/InputStream;", uri.object());
    qCInfo(documentDialog, "inputStream.isValid = %d", inputStream.isValid());
    if (!inputStream.isValid())
    {
        emit rejected();
        return;
    }

    jbyteArray bytes = env->NewByteArray(1024);
    //int InputSream.read(byte[] b, int off, int len)
    jint size = inputStream.callMethod<jint>("read", "([BII)I", bytes, 0, 1024);
    jint total = 0;
    while (size > 0)
    {
        total += size;
        size = inputStream.callMethod<jint>("read", "([BII)I", bytes, 0, 1024);
    }
    qCInfo(documentDialog, "total size: %d", total);

    //qDebug() << Q_FUNC_INFO << __LINE__;
    //setFileUrl(fileUrl);
    //qDebug() << Q_FUNC_INFO << __LINE__;
    */
    setFileUrl(uri.toString());
    emit accepted();
}
