#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H


class FrameTimer{
public:
	FrameTimer(){
		setup(0,0);
	}
	FrameTimer(float len_){
		setup(len_,0);
	}
	FrameTimer(float len_,float delay_){
		setup(len_,delay_);
	}
	void start(){
		started=true;
	}
	void stop(){
		started=false;
	}
	void reset(){
		started=false;
		ani_t=-delay;
	}
	void update(float dt_){
		if(!started) return;
		if(ani_t<due) ani_t+=dt_;
	}
	float val(){
		if(!started) return 0;
		if(ani_t<0) return 0;
		if(ani_t>=due) return 1;

		return ofClamp(ani_t/due,0,1);
	}
	int count(){
		if(ani_t<0) return ceil(abs(ani_t/1000.0));
		else return floor((due-ani_t)/1000.0);
	}
	void restart(){
		reset();
		start();
	}
private:
	float ani_t;
	float due,delay;
	bool started;
	void setup(float len_,float delay_){
		due=len_;
		delay=delay_;
		reset();
	}


};


#endif
