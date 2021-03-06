#include "jsmetatype.h"
#include <bgmrprocedure.h>
#include "jsobjectprototype.h"
#include "jsplugin.h"

using namespace BGMircroRPCServer;

QScriptValue jsonValueToScrObj(QScriptEngine* jsEngine,
                               const QJsonValue& jsonValue)
{
    QScriptValue scrValue;

//    QScriptValue JSON = jsEngine->globalObject ().property ("JSON");
//    QScriptValue parse = JSON.property ("parse");

    switch (jsonValue.type ()) {
    case QJsonValue::Bool:
        scrValue = jsonValue.toBool ();
        break;
    case QJsonValue::Double:
        scrValue = jsonValue.toDouble ();
        break;
    case QJsonValue::String:
        scrValue = jsonValue.toString ();
        break;
    case QJsonValue::Array:
    {
        scrValue = jsEngine->newObject ();
        QJsonArray theArray = jsonValue.toArray ();
        for (int i = 0; i < theArray.size (); ++i) {
            scrValue.setProperty (i, jsonValueToScrObj (jsEngine, theArray.at (i)));
        }
        break;
    }
    case QJsonValue::Object:
    {
        scrValue = jsEngine->newObject ();
        QJsonObject theObj = jsonValue.toObject ();
        QJsonObject::iterator it;
        for (it = theObj.begin (); it != theObj.end (); ++it) {
            scrValue.setProperty (it.key (), jsonValueToScrObj (jsEngine, it.value ()));
        }
        break;
    }
    case QJsonValue::Undefined:
        scrValue = jsEngine->undefinedValue ();
        break;
    case QJsonValue::Null:
        scrValue = jsEngine->nullValue ();
        break;
    }

    return scrValue;
}

void scrObjToJsonValue(const QScriptValue& scrValue, QJsonValue& jsonValue)
{
//    QScriptEngine* jsEngine = scrValue.engine ();

    if (scrValue.isNumber ())
        jsonValue = (double)scrValue.toNumber ();
    else if (scrValue.isBool ())
        jsonValue = scrValue.toBool ();
    else if (scrValue.isString ())
        jsonValue = scrValue.toString ();
    else if (scrValue.isObject () || scrValue.isArray ()) {
//        QScriptValue JSON = jsEngine->globalObject ().property ("JSON");
//        QScriptValue stringify = JSON.property ("stringify");
//        QString jsonContent
//                = stringify.call (JSON, QScriptValueList () << scrValue).toString ();
//        QJsonDocument jsonDoc = QJsonDocument::fromJson (jsonContent.toUtf8 ());
        QJsonDocument jsonDoc = QJsonDocument::fromVariant (scrValue.toVariant ());
        if (jsonDoc.isObject ())
            jsonValue = jsonDoc.object ();
        else
            jsonValue = jsonDoc.array ();
    }
}


QScriptValue jsonArrayToScrObj (QScriptEngine* jsEngine, const QJsonArray& jsonArray)
{
    return jsonValueToScrObj (jsEngine, jsonArray);
}

void scrObjToJsonArray(const QScriptValue& scrValue, QJsonArray& jsonArray)
{
    jsonArray = QJsonDocument::fromVariant (scrValue.toVariant ()).array ();
}


QScriptValue jsonObjToScrObj (QScriptEngine* jsEngine, const QJsonObject& jsonObj)
{
    return jsonValueToScrObj (jsEngine, jsonObj);
}

void scrObjToJsonObj (const QScriptValue& scrValue, QJsonObject& jsonObj)
{
    jsonObj = QJsonDocument::fromVariant (scrValue.toVariant ()).object ();
}


QScriptValue mapToScrObj(QScriptEngine* jsEngine, const QVariantMap& map)
{
    QScriptValue scrObj = jsEngine->newObject ();
    QVariantMap::ConstIterator it;
    for (it = map.constBegin (); it != map.constEnd (); ++it)
        scrObj.setProperty (it.key (), jsEngine->newVariant (it.value ()));

    return scrObj;
}


void scrObjToMap(const QScriptValue& scrObj, QVariantMap& map)
{
    map = scrObj.toVariant ().toMap ();
}

QScriptValue jsRelProcsToScrObj (QScriptEngine* jsEngine,
                                const relProcsMap& relProcs)
{
    QScriptValue scrObj = jsEngine->newObject ();
    QMap < qulonglong, BGMRProcedure* >::const_iterator it;
    for (it = relProcs.constBegin (); it != relProcs.constEnd (); ++it)
        scrObj.setProperty (QString::number (it.key ()),
                            jsEngine->toScriptValue(it.value ()));

    return scrObj;
}

void scrObjToJsRelProcs (const QScriptValue& scrObj, relProcsMap& relProcs)
{
    QScriptValueIterator it (scrObj);
    while (it.hasNext ()) {
        it.next ();
        BGMRProcedure* proc
                = scrObj.engine ()->fromScriptValue < BGMRProcedure* > (it.value ());
        if (!proc)
            continue;

        relProcs [it.name ().toULongLong ()] = proc;
    }
}

QScriptValue jsRPCObjListToScrObj(QScriptEngine* jsEngine,
                                  const RPCObjList& objList)
{
    QScriptValue obj = jsEngine->newObject ();

    RPCObjList::const_iterator it;
    for (it = objList.constBegin (); it != objList.constEnd (); ++it)
        obj.setProperty (it.key (), jsEngine->toScriptValue (it.value ()));

    return obj;
}

void scrObjToJsRPCObjList(const QScriptValue& scrObj, RPCObjList& objList)
{
    QScriptValueIterator it (scrObj);
    while (it.hasNext ()) {
        it.next ();
        BGMRObjectInterface* rpcObj
                = scrObj.engine ()->fromScriptValue < BGMRObjectInterface* > (it.value ());
        if (!rpcObj)
            continue;
        objList[rpcObj->objectName ()] = rpcObj;
    }
    //objList = qvariant_cast < RPCObjList > (scrObj.data ().toVariant ());
}

void scrObjToBGMRObj (const QScriptValue& scrObj, BGMRObjectInterface*& data)
{
    QVariant theVariant = scrObj.data ().toVariant ();
    if (scrObj.scriptClass ()->name () == "jsObj")
        data = qvariant_cast < jsObj* > (theVariant);
    else
        data = qvariant_cast < BGMRObjectInterface* > (theVariant);
}

void registerMetaType(QScriptEngine* jsEngine)
{
    qScriptRegisterMetaType < QJsonValue > (jsEngine, jsonValueToScrObj, scrObjToJsonValue);
    qScriptRegisterMetaType < QJsonArray > (jsEngine, jsonArrayToScrObj, scrObjToJsonArray);
    qScriptRegisterMetaType < QJsonObject > (jsEngine, jsonObjToScrObj, scrObjToJsonObj);

    qScriptRegisterMetaType < QVariantMap > (jsEngine, mapToScrObj, scrObjToMap);

    qRegisterMetaType < procPtr > ("BGMRProcedure*");
    qScriptRegisterMetaType < procPtr > (jsEngine, jsCustomDataToScrObj < jsProcProto >,
                                               scrObjToCustomData < jsProcProto >);
    qScriptRegisterMetaType < relProcsMap > (jsEngine, jsRelProcsToScrObj, scrObjToJsRelProcs);

    qScriptRegisterMetaType < rpcObjPtr > (jsEngine, jsCustomDataToScrObj < jsRPCObjectProto >,
                                           scrObjToBGMRObj);
    qScriptRegisterMetaType < RPCObjList > (jsEngine, jsRPCObjListToScrObj, scrObjToJsRPCObjList);
}
