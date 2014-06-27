#include <iostream>
#include <Kinect.h>
#include <vector>

using namespace std;

#define M_PI 3.141592

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

struct stpos
{
	float X;
	float Y;
	float Z;
};

struct index3D
{
	stpos index[1080][1920];
};

class KinectBasic
{
public:
	KinectBasic();
	~KinectBasic();

	IKinectSensor* pKinectSensor;
	IMultiSourceFrameReader* pMultiSourceFrameReader;
	ICoordinateMapper* pCoordinateMapper;

	unsigned short* pDepthBuffer;
	unsigned char* pDepthData;
	unsigned char* pColorData;
	unsigned short* pInfraredBuffer;
	unsigned char* pInfraredData;
	unsigned char* pBodyIndexData;
	RGBQUAD* pColorBuffer;

	CameraSpacePoint* pCameraSpacePoints;
	DepthSpacePoint* pDepthSpacePoints;
	ColorSpacePoint* pColorSpacePoints;

	index3D cp;

	INT64 nStartTime;
	INT64 nFrameCounter;

	bool oPickBodyIndex;
	bool oThresholdDepth;
	bool oThresholdInfrared;

	int iPickedBodyIndex;
	int iThresholdDepth;
	int iThresholdInfrared;

	static const int nDepthWidth;
	static const int nDepthHeight;
	static const int nColorWidth;
	static const int nColorHeight;
	static const int nInfraredWidth;
	static const int nInfraredHeight;
	static const int nDepthCount;
	static const int nColorCount;
	static const int nInfraredCount;

	HRESULT InitializeDefaultSensor();
	void ProcessFrame(
		INT64 nTime,
		const UINT16* pDepthSrc,
		const UINT16* pInfraredSrc,
		const BYTE* pBodyIndexSrc);
	void Update();

	void Toggle_PickBodyIndex(string& dispString);
	void Toggle_ThresholdDepthMode();
	void Toggle_ThresholdInfraredMode();
	void Set_PickedBodyIndex(const char bodyIndex, string& dispString);
};