using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace SerialManager
{
	internal class TimeTracker
	{
		private Stopwatch _stopwatch;

        public TimeTracker()
        {
            // Stopwatch 인스턴스 생성 및 시작
            _stopwatch = new Stopwatch();
            _stopwatch.Start();
        }

        public void Reset()
        {
            _stopwatch.Reset();
            _stopwatch.Start();
        }

        // 마이크로 초 반환
        public long GetElapsedTime()
        {
            return _stopwatch.ElapsedTicks / (Stopwatch.Frequency / 1000000);
        }
	}
}
