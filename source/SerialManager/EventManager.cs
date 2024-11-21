using System;
using System.Threading;

namespace SerialManager
{
	public class EventManager
    {
        internal delegate void EventCallback();
        internal delegate void EventCallbackWithChar(IntPtr ptr, ref int length);

        internal static SynchronizationContext mainThreadContext;

        internal EventManager()
        {
            mainThreadContext = SynchronizationContext.Current;
        }

        internal static void Invoke(Action action)
        {
            if (mainThreadContext == null) action?.Invoke();
            else mainThreadContext.Post(state => { action?.Invoke(); }, null);
        }

        internal static void Invoke(Action<double, SerialData> action, double time, SerialData data)
        {
            if (mainThreadContext == null) action?.Invoke(time, data);
            else mainThreadContext.Post(state => { action?.Invoke(time, data); }, null);
        }

		internal static void Invoke(Action<SerialLog> action, SerialLog input)
        {
            if (mainThreadContext == null) action?.Invoke(input);
            else mainThreadContext.Post(state => { action?.Invoke(input); }, null);
        }
    }
}
