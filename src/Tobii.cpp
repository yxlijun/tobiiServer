/*
* This is an example that demonstrates how to connect to the EyeX Engine and subscribe to the lightly filtered gaze data stream.
*
*/
//#include "stdafx.h"
#include "Tobii.h"

#define WIN32_LEAN_AND_MEAN

using namespace std;

// ID of the global interactor that provides our data stream; must be unique within the application.
static const TX_STRING InteractorId = "Twilight Sparkle";

// global variables
static TX_HANDLE g_hGlobalInteractorSnapshot = TX_EMPTY_HANDLE;

#define tobiiport "5557"

//保存tobii记录屏幕位置
cv::Point eyeAttention;

FILE *fp;

void *tobiipublisher;

//tobii context
TX_CONTEXTHANDLE hContext = TX_EMPTY_HANDLE;
TX_TICKET hConnectionStateChangedTicket = TX_INVALID_TICKET;
TX_TICKET hEventHandlerTicket = TX_INVALID_TICKET;

/*
* Initializes g_hGlobalInteractorSnapshot with an interactor that has the Gaze Point behavior.
*/

void DrawAttentionPicture(cv::Mat &eyeImage,const int x,const int y)
{
	CvScalar color;
	color = cvScalar(255, 255, 255);

	cv::Point eyeAttentionTmp;
	/*eyeAttentionTmp.x = r.width * eyeAttention.x / x;
	eyeAttentionTmp.y = r.height * eyeAttention.y / y;*/
	eyeAttentionTmp.x = eyeAttention.x;
	eyeAttentionTmp.y = eyeAttention.y; 
	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if (eyeAttention.x >=0 && eyeAttention.x <= x)
	{
		if (eyeAttention.y >= 0 && eyeAttention.y<=y)
		{
			circle(eyeImage, eyeAttentionTmp, 6, cv::Scalar(255, 0, 0), 6);
		}
	}

	imshow("test",eyeImage);
}
BOOL InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext)
{
	TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
	TX_GAZEPOINTDATAPARAMS params = { TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED };
	TX_FIXATIONDATAPARAMS fixationDataParams = { TX_FIXATIONDATAMODE_SENSITIVE };
	TX_HANDLE hBehaviorWithoutParameters = TX_EMPTY_HANDLE;
	BOOL success;

	success = txCreateGlobalInteractorSnapshot(
		hContext,
		InteractorId,
		&g_hGlobalInteractorSnapshot,
		&hInteractor) == TX_RESULT_OK;
	success &= txCreateGazePointDataBehavior(hInteractor, &params) == TX_RESULT_OK;

	// add a second behavior to the same interactor: fixation data
	success &= txCreateFixationDataBehavior(hInteractor, &fixationDataParams) == TX_RESULT_OK;

	// add a third behavior to the same interactor: eye position data.
	// this one is a bit different because it doesn't take any parameters.
	// therefore we use the generic txCreateInteractorBehavior function (and remember to release the handle!)
	success &= txCreateInteractorBehavior(hInteractor, &hBehaviorWithoutParameters, TX_BEHAVIORTYPE_EYEPOSITIONDATA) == TX_RESULT_OK;
	txReleaseObject(&hBehaviorWithoutParameters);

	txReleaseObject(&hInteractor);
	return success;
}

//获取当前系统的时间戳
int getSystemStamp(){
	/*time_t rawtime;
	//返回时间戳
	return time ( &rawtime );*/
	DWORD time;
	time = GetTickCount();
	return time;
}

/*
* Callback function invoked when a snapshot has been committed.
*/
void TX_CALLCONVENTION OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	// check the result code using an assertion.
	// this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
	assert(result == TX_RESULT_OK || result == TX_RESULT_CANCELLED);
}

/*
* Callback function invoked when the status of the connection to the EyeX Engine has changed.
*/
void TX_CALLCONVENTION OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
{
	switch (connectionState) {
	case TX_CONNECTIONSTATE_CONNECTED: {
		BOOL success;
		//printf("The connection state is now CONNECTED (We are connected to the EyeX Engine)\n");
		// commit the snapshot with the global interactor as soon as the connection to the engine is established.
		// (it cannot be done earlier because committing means "send to the engine".)
		success = txCommitSnapshotAsync(g_hGlobalInteractorSnapshot, OnSnapshotCommitted, NULL) == TX_RESULT_OK;
		if (!success) {
			printf("Failed to initialize the data stream.\n");
		}
		else
		{
			printf("Waiting for gaze data to start streaming...\n");
		}
	}
		break;

	case TX_CONNECTIONSTATE_DISCONNECTED:
		printf("The connection state is now DISCONNECTED (We are disconnected from the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_TRYINGTOCONNECT:
		printf("The connection state is now TRYINGTOCONNECT (We are trying to connect to the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
		printf("The connection state is now SERVER_VERSION_TOO_LOW: this application requires a more recent version of the EyeX Engine to run.\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
		printf("The connection state is now SERVER_VERSION_TOO_HIGH: this application requires an older version of the EyeX Engine to run.\n");
		break;
	}
}

/*
* Handles an event from the Gaze Point data stream.
*/
void OnGazeDataEvent(TX_HANDLE hGazeDataBehavior)
{
	TX_GAZEPOINTDATAEVENTPARAMS eventParams;

	if (txGetGazePointDataEventParams(hGazeDataBehavior, &eventParams) == TX_RESULT_OK) 
	{

		eyeAttention.x = eventParams.X;
		eyeAttention.y = eventParams.Y;

		//  向所有订阅者发送消息
		char update[20];
		sprintf(update, "%d %d", eyeAttention.x, eyeAttention.y);
		/*cout << "x: " << eyeAttention.x << endl;
		cout << "y: " << eyeAttention.y << endl;*/
		//TRACE("x: %d\r\n", eyeAttention.x);
		//TRACE("y: %d\r\n", eyeAttention.y);
		s_send(tobiipublisher, update);
	}

}

void OnFixationDataEvent(TX_HANDLE hGazeDataBehavior)
{
	TX_FIXATIONDATAEVENTPARAMS eventParams;
	TX_FIXATIONDATAEVENTTYPE eventType;
	char* eventDescription;
	if (txGetFixationDataEventParams(hGazeDataBehavior, &eventParams) == TX_RESULT_OK) {
		eventType = eventParams.EventType;

		eventDescription = (eventType == TX_FIXATIONDATAEVENTTYPE_DATA) ? "Data"
			: ((eventType == TX_FIXATIONDATAEVENTTYPE_END) ? "End"
			: "Begin");
		printf("Fixation %s: (%.1f, %.1f) timestamp %.0f ms\n", eventDescription, eventParams.X, eventParams.Y, eventParams.Timestamp);
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		fprintf(fp, "Fixation:%s,%.1f,%.1f,%.0f,%4d/%02d/%02d %02d:%02d:%02d.%03d,%d\n", eventDescription, eventParams.X, eventParams.Y, eventParams.Timestamp, sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, getSystemStamp());
	}
	else {
		printf("Failed to interpret fixation data event packet.\n");
	}

}

void OnEyeDataEvent(TX_HANDLE hGazeDataBehavior)
{
	TX_EYEPOSITIONDATAEVENTPARAMS eventParams;
	if (txGetEyePositionDataEventParams(hGazeDataBehavior, &eventParams) == TX_RESULT_OK) {
		printf("Eye Data: <left_eye, right_eye>  <%d,%d>\n", eventParams.HasLeftEyePosition, eventParams.HasRightEyePosition);
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		fprintf(fp, "Eye:%d,%d,%.0f,%4d/%02d/%02d %02d:%02d:%02d.%03d,%d\n", eventParams.HasLeftEyePosition, eventParams.HasRightEyePosition, eventParams.Timestamp, sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, getSystemStamp());
	}
	else {
		printf("Failed to interpret gaze data event packet.\n");
		printf("Error code: %d.\n", txGetEyePositionDataEventParams(hGazeDataBehavior, &eventParams));
	}
}

/*
* Callback function invoked when an event has been received from the EyeX Engine.
*/
void TX_CALLCONVENTION HandleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam)
{
	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;

	txGetAsyncDataContent(hAsyncData, &hEvent);

	// NOTE. Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
	//OutputDebugStringA(txDebugObject(hEvent));


	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK) {
		OnGazeDataEvent(hBehavior);
		txReleaseObject(&hBehavior);
	}

	/*暂时不需要fixation数据和EYE数据*/
	/*
	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_EYEPOSITIONDATA) == TX_RESULT_OK) {
	OnEyeDataEvent(hBehavior);
	txReleaseObject(&hBehavior);
	}


	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_FIXATIONDATA) == TX_RESULT_OK) {
	OnFixationDataEvent(hBehavior);
	txReleaseObject(&hBehavior);
	}
	*/
	// NOTE since this is a very simple application with a single interactor and a single data stream, 
	// our event handling code can be very simple too. A more complex application would typically have to 
	// check for multiple behaviors and route events based on interactor IDs.

	txReleaseObject(&hEvent);
}

BOOL tobii_init()
{
	BOOL success;

	// initialize and enable the context that is our link to the EyeX Engine.
	success = txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, NULL, NULL, NULL, NULL) == TX_RESULT_OK;
	success &= txCreateContext(&hContext, TX_FALSE) == TX_RESULT_OK;
	success &= InitializeGlobalInteractorSnapshot(hContext);
	success &= txRegisterConnectionStateChangedHandler(hContext, &hConnectionStateChangedTicket, OnEngineConnectionStateChanged, NULL) == TX_RESULT_OK;
	success &= txRegisterEventHandler(hContext, &hEventHandlerTicket, HandleEvent, NULL) == TX_RESULT_OK;
	success &= txEnableConnection(hContext) == TX_RESULT_OK;

	void *context = zmq_init(1);
	tobiipublisher = zmq_socket(context, ZMQ_PUB);
	cv::string port(tobiiport);
	int rc = zmq_bind(tobiipublisher, ("tcp://*:" + port).c_str());
	assert(rc == 0);
	cout << success << endl;
	return success;
}

void tobii_uninit()
{
	// disable and delete the context.
	txDisableConnection(hContext);
	txReleaseObject(&g_hGlobalInteractorSnapshot);
	txShutdownContext(hContext, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
	txReleaseContext(&hContext);
}