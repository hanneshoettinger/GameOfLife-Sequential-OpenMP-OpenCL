
class TimerFunc {
	public:
		TimerFunc();
		~TimerFunc();
		void startTimer();
		void stopTimer(int measure);
	private:
		LARGE_INTEGER frequency;        // ticks per second
		LARGE_INTEGER t1, t2;           // ticks
		double elapsedTime;
};