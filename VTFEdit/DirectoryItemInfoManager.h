#pragma once

#include "stdafx.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace VTFEdit
{
	struct SFileInfo
	{
		HICON hIcon;
		INT iIcon;
		DWORD dwAttributes;
		TCHAR szDisplayName[MAX_PATH];
		TCHAR szTypeName[80];
	};

	enum EFileInfoFlags
	{
		eIcon                = 0x000000100,     // get icon
		eDisplayName         = 0x000000200,     // get display name
		eTypeName            = 0x000000400,     // get type name
		eAttributes          = 0x000000800,     // get attributes
		eIconLocation        = 0x000001000,     // get icon location
		eExeType             = 0x000002000,     // return exe type
		eSystemIconIndex     = 0x000004000,     // get system icon index
		eLinkOverlay         = 0x000008000,     // put a link overlay on icon
		eSelected            = 0x000010000,     // show icon in selected state
		eAttributesSpecified = 0x000020000,     // get only specified attributes
		eLargeIcon           = 0x000000000,     // get large icon
		eSmallIcon           = 0x000000001,     // get small icon
		eOpenIcon            = 0x000000002,     // get open icon
		eShellIconSize       = 0x000000004,     // get shell size icon
		ePIDL                = 0x000000008,     // pszPath is a pidl
		eUseFileAttributes   = 0x000000010     // use passed dwFileAttribute
	};

	[DllImport("Shell32.dll")]
	extern int SHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SFileInfo *FileInfo, UINT cbFileInfo, UINT uFlags);

	ref class CDirectoryItemInfo
	{
	private:
		String^ sType;
		String^ sDescription;

		int iIconIndex;

	public:
		CDirectoryItemInfo(String^ sType, String^ sDescription, int iIconIndex)
		{
			this->sType = sType;
			this->sDescription = sDescription;

			this->iIconIndex = iIconIndex;
		}

		property String^ Type
		{
			String^ get() { return this->sType; }
		}

		property String^ Description
		{
			String^ get() { return this->sDescription; }
		}

		property int IconIndex
		{
			int get() { return this->iIconIndex; }
		}
	};

	ref class CDirectoryItemInfoManager
	{
	private:
		bool bFileMapping;
		bool bVolatileAccess;

	private:
		System::Collections::Hashtable^ oFileTypeInfoTable;

	private:
		System::Windows::Forms::ImageList^ oSmallImageList;

	public:
		CDirectoryItemInfoManager()
		{
			this->bFileMapping = true;
			this->bVolatileAccess = true;

			this->oFileTypeInfoTable = gcnew System::Collections::Hashtable();

			this->oSmallImageList = gcnew System::Windows::Forms::ImageList();
			this->oSmallImageList->ImageSize = System::Drawing::Size(16, 16);
			this->oSmallImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
			this->oSmallImageList->TransparentColor = System::Drawing::Color::Transparent;
		}

	public:
		property bool FileMapping
		{
			bool get() { return this->bFileMapping; }
			void set(bool v) { this->bFileMapping = v; }
		}

		property bool VolatileAccess
		{
			bool get() { return this->bVolatileAccess; }
			void set(bool v) { this->bVolatileAccess = v; }
		}

	public:
		property ImageList^ SmallImageList
		{
			ImageList^ get() { return this->oSmallImageList; }
		}

	public:
		CDirectoryItemInfo^ GetFileTypeInfo(String^ sFile)
		{
			String^ sFileType;
			if(sFile->LastIndexOf(".") != -1)
			{
				sFileType = sFile->Substring(sFile->LastIndexOf(".") + 1)->ToUpper();
			}
			else
			{
				sFileType = "File";
			}

			return this->GetItemTypeInfo(sFile, sFileType, FILE_ATTRIBUTE_NORMAL);
		}

		CDirectoryItemInfo^ GetFolderTypeInfo(String^ sFolder, String^ sFolderType)
		{
			return this->GetItemTypeInfo(sFolder, sFolderType, FILE_ATTRIBUTE_DIRECTORY);
		}

		CDirectoryItemInfo^ GetFolderTypeInfo(String^ sFolderType)
		{
			return this->GetItemTypeInfo("Folder", sFolderType, FILE_ATTRIBUTE_DIRECTORY);
		}

	private:
		CDirectoryItemInfo^ GetItemTypeInfo(String^ sItemPath, String^ sItemType, DWORD dwFileAttributes)
		{
			//String *sFolderType = S"Folder";

			Object^ oValue = this->oFileTypeInfoTable[sItemType];
			if(oValue == nullptr)
			{
				SFileInfo FileInfo;

				String^ sDescription;
				int iIconIndex;
				System::Drawing::Icon^ oSmallIcon;
				System::Drawing::Icon^ oSmallIconOpen;

				char *cItemPath = (char *)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(sItemPath)).ToPointer();

				if(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					sDescription = sItemType;
				}
				else
				{
					if(SHGetFileInfo(cItemPath, dwFileAttributes, &FileInfo, sizeof(FileInfo), eTypeName | eUseFileAttributes))
					{
						sDescription = gcnew String(FileInfo.szTypeName);
					}
					else
					{
						sDescription = sItemType;
					}
				}

				SHGetFileInfo(cItemPath, dwFileAttributes, &FileInfo, sizeof(FileInfo), eIcon | eSmallIcon | eUseFileAttributes);
				oSmallIcon = System::Drawing::Icon::FromHandle((IntPtr)FileInfo.hIcon);

				this->oSmallImageList->Images->Add(oSmallIcon);

				iIconIndex = this->oSmallImageList->Images->Count - 1;

				if(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(SHGetFileInfo(cItemPath, dwFileAttributes, &FileInfo, sizeof(FileInfo), eIcon | eSmallIcon | eOpenIcon | eUseFileAttributes))
					{
						oSmallIconOpen = System::Drawing::Icon::FromHandle((IntPtr)FileInfo.hIcon);
					}
					else
					{
						oSmallIconOpen = oSmallIcon;
					}

					this->oSmallImageList->Images->Add(oSmallIconOpen);
				}

				System::Runtime::InteropServices::Marshal::FreeHGlobal((IntPtr)cItemPath);

				CDirectoryItemInfo^ ItemTypeInfo = gcnew CDirectoryItemInfo(sItemType, sDescription, iIconIndex);

				this->oFileTypeInfoTable->Add(sItemType, ItemTypeInfo);

				return ItemTypeInfo;
			}
			else
			{
				return static_cast<CDirectoryItemInfo^>(oValue);
			}
		}
	};
}