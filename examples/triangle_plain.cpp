#include <vtek/vtek.h>


int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.applicationTitle = "exTrianglePlain";
	vtek::initialize(&initInfo);


	vtek::terminate();

	return 0;
}
