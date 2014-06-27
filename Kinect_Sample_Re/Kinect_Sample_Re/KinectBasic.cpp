#include <gl\freeglut.h>		// OpenGL header files
#include "KinectBasic.h"

const int KinectBasic::nDepthWidth = 512;
const int KinectBasic::nDepthHeight = 424;
const int KinectBasic::nColorWidth = 1920;
const int KinectBasic::nColorHeight = 1080;
const int KinectBasic::nInfraredWidth = KinectBasic::nDepthWidth;
const int KinectBasic::nInfraredHeight = KinectBasic::nDepthHeight;
const int KinectBasic::nDepthCount = nDepthWidth * nDepthHeight;
const int KinectBasic::nColorCount = nColorWidth * nColorHeight;
const int KinectBasic::nInfraredCount = nInfraredWidth * nInfraredHeight;

KinectBasic::KinectBasic() :
pKinectSensor(NULL),
pCoordinateMapper(NULL),
pMultiSourceFrameReader(NULL),
pDepthBuffer(NULL),
pDepthData(NULL),
pColorBuffer(NULL),
pColorData(NULL),
pInfraredBuffer(NULL),
pInfraredData(NULL),
pCameraSpacePoints(NULL),
pColorSpacePoints(NULL),
pDepthSpacePoints(NULL),
oPickBodyIndex(false),
oThresholdDepth(true),
oThresholdInfrared(true),
iPickedBodyIndex(255),
iThresholdDepth(1200),
iThresholdInfrared(4000),
nStartTime(0),
nFrameCounter(0)
{
	pDepthBuffer = new unsigned short[nDepthCount];
	pDepthData = new unsigned char[nDepthCount];
	pColorBuffer = new RGBQUAD[nColorCount];
	pColorData = new unsigned char[nColorCount * 3];
	pInfraredBuffer = new unsigned short[nInfraredCount];
	pInfraredData = new unsigned char[nInfraredCount];

	pCameraSpacePoints = new CameraSpacePoint[nColorCount];
	pDepthSpacePoints = new DepthSpacePoint[nDepthCount];

	memset(pDepthBuffer, 0, sizeof(unsigned short)* nDepthCount);
	memset(pDepthData, 0, sizeof(unsigned char)* nDepthCount);
	memset(pColorBuffer, 0, sizeof(RGBQUAD)* nColorCount);
	memset(pColorData, 0, sizeof(unsigned char)* nColorCount * 3);
	memset(pInfraredBuffer, 0, sizeof(unsigned short)* nInfraredCount);
	memset(pInfraredData, 0, sizeof(unsigned char)* nInfraredCount);
	memset(pCameraSpacePoints, 0, sizeof(CameraSpacePoint)* nColorCount);
	memset(pDepthSpacePoints, 0, sizeof(DepthSpacePoint)* nDepthCount);

}

KinectBasic::~KinectBasic()
{
	if (pDepthBuffer != NULL)	delete[] pDepthBuffer;
	if (pDepthData != NULL)		delete[] pDepthData;
	if (pColorBuffer != NULL)	delete[] pColorBuffer;
	if (pColorData != NULL)		delete[] pColorData;
	if (pInfraredBuffer != NULL)	delete[] pInfraredBuffer;
	if (pInfraredData != NULL)	delete[] pInfraredData;

	if (pCameraSpacePoints != NULL)	delete[] pColorSpacePoints;
	if (pDepthSpacePoints != NULL)	delete[] pDepthSpacePoints;

	SafeRelease(pCoordinateMapper);
	SafeRelease(pMultiSourceFrameReader);

	if (pKinectSensor != NULL)	pKinectSensor->Close();
	SafeRelease(pKinectSensor);
}

HRESULT KinectBasic::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the fream reader
		if (SUCCEEDED(hr))
		{
			hr = pKinectSensor->get_CoordinateMapper(&pCoordinateMapper);
		}

		if (SUCCEEDED(hr))
		{
			hr = pKinectSensor->Open();
		}

		if (SUCCEEDED(hr))
		{
			hr = pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Depth |
				FrameSourceTypes::FrameSourceTypes_Color |
				FrameSourceTypes::FrameSourceTypes_Infrared |
				FrameSourceTypes::FrameSourceTypes_BodyIndex,
				&pMultiSourceFrameReader);
		}
	}

	if (pKinectSensor == NULL || FAILED(hr))
	{
		cerr << "No ready Kinect found" << endl;
		return E_FAIL;
	}

	return hr;
}

void KinectBasic::ProcessFrame(
	INT64 nTime,
	const UINT16* pDepthSrc,
	const UINT16* pInfraredSrc,
	const BYTE* pBodyIndexSrc)
{
	// process time
	if (nStartTime == 0) nStartTime = nTime;
	else if (nStartTime != nTime)
	{
		nFrameCounter++;
	}

	memcpy(pDepthBuffer, pDepthSrc, sizeof(UINT16)* nDepthCount);

	// copy infrared data
	memcpy(pInfraredBuffer, pInfraredSrc, sizeof(UINT16)* nInfraredCount);

	// thresholding depth data by depth
	if (oThresholdDepth)
	{
		for (int register ii = 0; ii < nDepthCount; ii++)
		if (pDepthBuffer[ii] < 0 || pDepthBuffer[ii] > iThresholdDepth)
			pDepthBuffer[ii] = 0;
	}

	// thresholding depth data by infrared
	if (oThresholdInfrared)
	{
		for (int register ii = 0; ii < nDepthCount; ii++)
		{
			if (pInfraredBuffer[ii] < iThresholdInfrared)
				pDepthBuffer[ii] = 0;
		}
	}

	HRESULT hr;

	// convert points to camera space
	hr = pCoordinateMapper->MapColorFrameToCameraSpace(
		nDepthCount,
		pDepthBuffer,
		nColorCount,
		pCameraSpacePoints);


	// recompute x, y using z and focal length
	//focal length [ 1063.018  1065.133 ] ¡¾ [ 1.880  1.889 ]
	//principal point [ 962.373  526.689 ] ¡¾ [ 1.085  0.885 ]
	//distortion [ 0.042369  -0.037696  -0.002894  0.000978 ] ¡¾ [ 0.002178  0.009347  0.000238  0.000308 ]
	float fl_x = 1063.118;
	float fl_y = 1065.233;
	float pp_x = 962.473;
	float pp_y = 526.789;

	for (int register rr = 0; rr < nColorHeight; rr++)
	for (int register cc = 0; cc < nColorWidth; cc++)
	{
		float Z = pCameraSpacePoints[rr * nColorWidth + cc].Z;
		cp.index[rr][cc].Z = Z;
		if (Z > 0)
		{
			cp.index[rr][cc].X = (cc - pp_x) * Z / fl_x;
			cp.index[rr][cc].Y = -(rr - pp_y) * Z / fl_y;
		}
	}
	
	if (SUCCEEDED(hr))
	{
		// process depth
		for (int ii = 0; ii < nDepthCount; ii++)
		{
			pDepthBuffer[ii] = (pDepthBuffer[ii] & 0xfff8) >> 3;
			pDepthData[ii] = pDepthBuffer[ii] % 256;
		}

		// process infrared
		for (int ii = 0; ii < nInfraredCount; ii++)
		{
			pInfraredBuffer[ii] = pInfraredBuffer[ii] >> 8;
			pInfraredData[ii] = pInfraredBuffer[ii] % 256;
		}

		// process color: convert to RGB
		int idx_char = 0;
		int idx_quad = 0;
		for (int rr = 0; rr < nColorHeight; rr++)
		{
			for (int cc = 0; cc < nColorWidth; cc++, idx_quad++)
			{
				this->pColorData[idx_char++] = pColorBuffer[idx_quad].rgbBlue;
				this->pColorData[idx_char++] = pColorBuffer[idx_quad].rgbGreen;
				this->pColorData[idx_char++] = pColorBuffer[idx_quad].rgbRed;
			}
		}
	}
	else cout << "ProcessFrame failed." << endl;
}

void KinectBasic::Update()
{
	if (pMultiSourceFrameReader == NULL)
	{
		return;
	}

	IMultiSourceFrame* pMultiSourceFrame = NULL;
	IDepthFrame* pDepthFrame = NULL;
	IColorFrame* pColorFrame = NULL;
	IInfraredFrame* pInfraredFrame = NULL;
	IBodyIndexFrame* pBodyIndexFrame = NULL;

	HRESULT hr = pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);
	if (FAILED(hr))
		printf("AcquireLatestFrame(&pMultiSourceFrame) failed.\n");

	// acquire depth frame
	if (SUCCEEDED(hr))
	{
		IDepthFrameReference* pDepthFrameReference = NULL;

		hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthFrameReference);
		if (FAILED(hr))
			printf("get_DepthFrameReference(&pDepthFrameReference) failed.\n");

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
			if (FAILED(hr))
				printf("AcquireFrame(&pDepthFrame) failed.\n");
		}

		SafeRelease(pDepthFrameReference);
	}

	// acquire color frame
	if (SUCCEEDED(hr))
	{
		IColorFrameReference* pColorFrameReference = NULL;

		hr = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
		if (FAILED(hr))
			printf("get_ColorFrameReference(&pColorFrameReference) failed.\n");

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameReference->AcquireFrame(&pColorFrame);
			if (FAILED(hr))
				printf("AcquireFrame(&pColorFrame) failed.\n");
		}

		SafeRelease(pColorFrameReference);
	}

	// acquire infrared frame
	if (SUCCEEDED(hr))
	{
		IInfraredFrameReference* pInfraredFrameReference = NULL;

		hr = pMultiSourceFrame->get_InfraredFrameReference(&pInfraredFrameReference);
		if (FAILED(hr))
			printf("get_InfraredFrameReference(&pInfraredFrameReference) failed.\n");

		if (SUCCEEDED(hr))
		{
			hr = pInfraredFrameReference->AcquireFrame(&pInfraredFrame);
			if (FAILED(hr))
				printf("AcquireFrame(&pInfraredFrame) failed.\n");
		}

		SafeRelease(pInfraredFrameReference);
	}

	// acquire body index frame
	if (SUCCEEDED(hr))
	{
		IBodyIndexFrameReference* pBodyIndexFrameReference = NULL;

		hr = pMultiSourceFrame->get_BodyIndexFrameReference(&pBodyIndexFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameReference->AcquireFrame(&pBodyIndexFrame);
		}

		SafeRelease(pBodyIndexFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		INT64 nDepthTime = 0;
		IFrameDescription* pDepthFrameDescription = NULL;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxReliableDistance = 0;
		int nDepthWidth = 0;
		int nDepthHeight = 0;
		UINT nDepthBufferSize = 0;
		UINT16 *pDepthBuffer = NULL;

		IFrameDescription* pColorFrameDescription = NULL;
		int nColorWidth = 0;
		int nColorHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;

		IFrameDescription* pInfraredFrameDescription = NULL;
		int nInfraredWidth = 0;
		int nInfraredHeight = 0;
		UINT nInfraredBufferSize = 0;
		UINT16* pInfraredBuffer = NULL;

		IFrameDescription* pBodyIndexFrameDescription = NULL;
		int nBodyIndexWidth = 0;
		int nBodyIndexHeight = 0;
		UINT nBodyIndexBufferSize = 0;
		BYTE* pBodyIndexBuffer = NULL;

		// get depth frame data
		{
			hr = pDepthFrame->get_RelativeTime(&nDepthTime);

			if (SUCCEEDED(hr))
				hr = pDepthFrame->get_FrameDescription(&pDepthFrameDescription);

			if (SUCCEEDED(hr))
				hr = pDepthFrameDescription->get_Width(&nDepthWidth);

			if (SUCCEEDED(hr))
				hr = pDepthFrameDescription->get_Height(&nDepthHeight);

			if (SUCCEEDED(hr))
				hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);

			if (SUCCEEDED(hr))
				hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
				if (FAILED(hr))
					printf("AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer) failed.\n");

			}
		}

		// get color frame data
		{
			if (SUCCEEDED(hr))
				hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);

			if (SUCCEEDED(hr))
				hr = pColorFrameDescription->get_Width(&nColorWidth);

			if (SUCCEEDED(hr))
				hr = pColorFrameDescription->get_Height(&nColorHeight);

			if (SUCCEEDED(hr))
				hr = pColorFrame->get_RawColorImageFormat(&imageFormat);

			if (SUCCEEDED(hr))
			{
				hr = pColorFrame->CopyConvertedFrameDataToArray(
					nColorCount * 4,
					reinterpret_cast<BYTE*>(pColorBuffer),
					ColorImageFormat_Rgba);
				if (FAILED(hr))
					printf("CopyConvertedFrameDataToArray(pColorBuffer) failed.\n");
			}
		}

		// get infrared frame data
		{
			if (SUCCEEDED(hr))
				hr = pInfraredFrame->get_FrameDescription(&pInfraredFrameDescription);

			if (SUCCEEDED(hr))
				hr = pInfraredFrameDescription->get_Width(&nInfraredWidth);

			if (SUCCEEDED(hr))
				hr = pInfraredFrameDescription->get_Height(&nInfraredHeight);

			if (SUCCEEDED(hr))
			{
				hr = pInfraredFrame->AccessUnderlyingBuffer(&nInfraredBufferSize, &pInfraredBuffer);
				if (FAILED(hr))
					printf("AccessUnderlyingBuffer(&nInfraredBufferSize, &pInfraredBuffer) failed.\n");
			}
		}

		// get body index frame data
		{
			if (SUCCEEDED(hr))
			{
				hr = pBodyIndexFrame->get_FrameDescription(&pBodyIndexFrameDescription);
			}

			if (SUCCEEDED(hr))
			{
				hr = pBodyIndexFrameDescription->get_Width(&nBodyIndexWidth);
			}

			if (SUCCEEDED(hr))
			{
				hr = pBodyIndexFrameDescription->get_Height(&nBodyIndexHeight);
			}

			if (SUCCEEDED(hr))
			{
				hr = pBodyIndexFrame->AccessUnderlyingBuffer(&nBodyIndexBufferSize, &pBodyIndexBuffer);
			}
		}

		if (SUCCEEDED(hr))
		{
			ProcessFrame(
				nDepthTime,
				pDepthBuffer,
				pInfraredBuffer,
				pBodyIndexBuffer);
		}
		else cout << "bad" << endl;

		SafeRelease(pDepthFrameDescription);
		SafeRelease(pColorFrameDescription);
		SafeRelease(pInfraredFrameDescription);
		SafeRelease(pBodyIndexFrameDescription);
	}

	SafeRelease(pDepthFrame);
	SafeRelease(pColorFrame);
	SafeRelease(pInfraredFrame);
	SafeRelease(pBodyIndexFrame);
	SafeRelease(pMultiSourceFrame);
}

void KinectBasic::Toggle_PickBodyIndex(string& dispString)
{
	this->oPickBodyIndex = !this->oPickBodyIndex;
	if(this->oPickBodyIndex == true)
	Set_PickedBodyIndex('0', dispString);
	else dispString = "";
}

void KinectBasic::Toggle_ThresholdDepthMode()
{
	this->oThresholdDepth = !this->oThresholdDepth;
}

void KinectBasic::Toggle_ThresholdInfraredMode()
{
	this->oThresholdInfrared = !this->oThresholdInfrared;
}

void KinectBasic::Set_PickedBodyIndex(const char bodyKey, string& dispString)
{
	if(bodyKey >= '0' && bodyKey <= '9' && this->oPickBodyIndex)
	{
	this->iPickedBodyIndex = bodyKey - '0';
	char buff[1024];
	sprintf_s(buff, "PickedBodyIndex: %d", bodyKey - '0');
	dispString = buff;
	}
}
