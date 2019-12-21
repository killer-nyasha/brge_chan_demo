#pragma once
#include "metagen_def.h"
#include "metagen_classes.h"
#include "Print.h"

#include <initializer_list>

#define FIELD(field) rawFieldInfo(&(sample->field), #field)
#define FUNC(...) rawFieldInfo(&__VA_ARGS__::
#define WITHNAME(fname) fname, #fname)
#define PARAMS(...) WithParams<__VA_ARGS__>

#define METAGEN_CLASS(className,...) ([](className* sample) { FromParents<__VA_ARGS__>::createMetadata(sample
#define WITH_MEMBERS(...) __VA_ARGS__); })(nullptr)

#define FORVARIADIC(...) std::initializer_list<int>({ (__VA_ARGS__, 0)... });

namespace LizardScript
{
	template <typename T>
	struct RawFieldInfo
	{
		void* offset;
		ptrbuf funcptr;
		std::string name;
	};

	template <typename T>
	inline RawFieldInfo<T> rawFieldInfo(T* ptr, std::string name)
	{
		RawFieldInfo<T> r;
		r.offset = (void*)ptr;
		r.name = name;
		return r;
	}

	template <typename R, typename O, typename... A>
	inline RawFieldInfo<R(O::*)(A...)> rawFieldInfo(R(O::*ptr)(A...), std::string name)
	{
		ptrbuf buffer;
		memcpy(&buffer, &ptr, sizeof(ptr) + sizeof(void*));
		return RawFieldInfo<R(O::*)(A...)> { (void*)-1, buffer, name };
	}

	template <typename... A>
	struct WithParams
	{
		template <typename R, typename O>
		static inline RawFieldInfo<R(O::*)(A...)> rawFieldInfo(R(O::*ptr)(A...), std::string name)
		{
			ptrbuf buffer;
			memcpy(&buffer, &ptr, sizeof(ptr) + sizeof(void*));
			return RawFieldInfo<R(O::*)(A...)> { (void*)-1, buffer, name };
		}

		template <typename R, typename O>
		static inline RawFieldInfo<R(O::*)(A...)> rawFieldInfo(R(O::*ptr)(A...) const, std::string name)
		{
			ptrbuf buffer;
			memcpy(&buffer, &ptr, sizeof(ptr) + sizeof(void*));
			return RawFieldInfo<R(O::*)(A...)> { (void*)-1, buffer, name };
		}
	};

	template <typename T>
	void addSimpleTypeInfo()
	{
		TypeInfo t = TYPEINFO(T);
		TypeInfoEx tex = TypeInfoEx(t);
		if (globalMetadataTable.count(t) == 0)
			globalMetadataTable.insert(std::make_pair(t, tex));
	}

	template <typename T>
	inline FieldInfo createMetadataEntry(RawFieldInfo<T> info)
	{
		FieldInfo f;
		f.type = makeTypeInfo<T>();
		f.name = info.name;
		f.offset = (int)info.offset;
		addSimpleTypeInfo<T>();
		return f;
	}

	template <typename O, typename R, typename... A>
	inline FunctionInfo createMetadataEntry(RawFieldInfo<R(O::*)(A...)> info)
	{
		CallStruct<O, R, A...> callStruct;
		callStruct.funcptr = *reinterpret_cast<R(O::**)(A...)>(&info.funcptr);
		FunctionInfo f;
		f.type = makeTypeInfo<R>();
		f.args = { (makeTypeInfo<A>())... };

		//��� �������� ������� �������� �� ��������
		for (auto& a : f.args)
			if (a.byValueSize > sizeof(void*) && a.ptr == 0)
				a.ptr = 1;

		f.name = info.name;
		memcpy(&f.callStruct, &callStruct, sizeof(callStruct));

		addSimpleTypeInfo<R>();
		//addSimpleTypeInfo<O>();
		FORVARIADIC(addSimpleTypeInfo<A>());
		return f;
	}

	template <typename... P>
	class FromParents
	{
	public:
		template <typename O, typename... T>
		static inline void createMetadata(O* object, T... infos)
		{
			TypeInfo info = makeTypeInfo<O>();
			TypeInfoEx einfo = TypeInfoEx(info);
			FORVARIADIC(einfo.parents.push_back(ParentInfo { (size_t)static_cast<P*>(object + 10000000) - (size_t)(object + 10000000), TYPEINFO(P) }));
			FORVARIADIC(einfo.members.push_back(createMetadataEntry(infos)), '\0');
			if (globalMetadataTable.count(info) > 0)
				globalMetadataTable[info] = einfo;
			else globalMetadataTable.insert(std::make_pair(info, einfo));
		}
	};

	inline void processParents(const TypeInfo& type, TypeInfoEx& typeex)
	{
		if (!typeex.parentsProcessed)
		{
			for (auto& parent : typeex.parents)
				if (type != parent.type && globalMetadataTable.count(parent.type) > 0)
				{
					processParents(parent.type, globalMetadataTable[parent.type]);
					typeex.addParentMembers(globalMetadataTable[parent.type], parent.offset);
				}
			typeex.parentsProcessed = true;
		}
	}

	inline void endMetadata()
	{
		for (auto& type : globalMetadataTable)
			processParents(type.first, type.second);
	}

}

