#include "Print.h"

//using namespace LizardScript;

namespace LizardScript
{
	void print(std::ostream& stream, char* object, TypeInfo currentType)
	{
		auto& metatable = globalMetadataTable[currentType];

		stream << COLOR_BLUE << currentType.t.name() << COLOR_NC << " {\r\n";

		for (auto& metadata : metatable.members.get<FunctionInfo>())
		{
			stream << COLOR_BLUE << metadata.type.text() << " " << COLOR_NC << metadata.name << "(";
			bool needComma = false;
			for (auto& t : metadata.args)
			{
				if (!needComma)
					needComma = true;
				else stream << ", ";
				stream << COLOR_BLUE << t.text() << COLOR_NC;
			}
			stream << ");" << ENDL;
		}
		for (auto& metadata : metatable.members.get<FieldInfo>())
		{
			stream << COLOR_BLUE << metadata.type.text() << COLOR_NC << " " << metadata.name;

			if (metadata.type == makeTypeInfo<int>())
			{
				int* p = (int*)(object + metadata.offset);
				for (size_t i = 0; i < metadata.type.ptr; i++)
					p = *((int**)p);
				stream << " = " << *p << ";" << ENDL;
			}
			else if (metadata.type == makeTypeInfo<float>())
			{
				int* p = (int*)(object + metadata.offset);
				for (size_t i = 0; i < metadata.type.ptr; i++)
					p = *((int**)p);
				stream << " = " << *(float*)p << ";" << ENDL;
			}
			else if(metadata.type.ptr > 0)
			{
				char* objPtr = *(char**)(object + metadata.offset);

				stream << " = ";
				if (objPtr == nullptr)
					stream << COLOR_BLUE << "null;" << ENDL << COLOR_NC;
				else
				{
					//����� �� �����??
					//for (size_t i = 0; i < metadata.type.ptr; i++)
					//	objPtr = *((char**)objPtr);
					print(stream, objPtr, metadata.type);
				}
			}
			else
			{
				char* objPtr = (char*)(object + metadata.offset);
				stream << " = ";
				print(stream, objPtr, metadata.type);
			}
		}

		stream << "}" << ENDL;
	}

}