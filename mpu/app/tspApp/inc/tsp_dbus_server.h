#ifndef  _TSP_DBUS_SERVER_H__
#define  _TSP_DBUS_SERVER_H__

gpointer RemoteDiagTaskEntry(gpointer argv);

gint EmitTspEvent(const gchar* payload, gint length);

#endif		
