// ===========================================================
//     _/_/_/   _/_/_/  _/_/_/_/    _/_/_/_/  _/_/_/   _/_/_/
//      _/    _/       _/             _/    _/    _/   _/   _/
//     _/    _/       _/_/_/_/       _/    _/    _/   _/_/_/
//    _/    _/       _/             _/    _/    _/   _/
// _/_/_/   _/_/_/  _/_/_/_/_/     _/     _/_/_/   _/_/
// ===========================================================
//
// dFile.h: Functions and classes for CoreArray Genomic Data Structure (GDS)
//
// Copyright (C) 2007-2015    Xiuwen Zheng
//
// This file is part of CoreArray.
//
// CoreArray is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License Version 3 as
// published by the Free Software Foundation.
//
// CoreArray is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with CoreArray.
// If not, see <http://www.gnu.org/licenses/>.

/**
 *	\file     dFile.h
 *	\author   Xiuwen Zheng [zhengx@u.washington.edu]
 *	\version  1.0
 *	\date     2007 - 2015
 *	\brief    Functions and classes for CoreArray Genomic Data Structure (GDS)
 *	\details
**/

#ifndef _HEADER_COREARRAY_FILE_
#define _HEADER_COREARRAY_FILE_

#include "dBase.h"
#include "dSerial.h"
#include "dStream.h"
#include "dAny.h"


namespace CoreArray
{
	// =====================================================================
	// CdGDSObj
	// =====================================================================

	class CdGDSObj;
	class CdGDSFolder;
	class CdGDSFile;

	/// Attribute class for GDS object
	class COREARRAY_DLL_DEFAULT CdObjAttr: public CdObject
	{
	public:
		friend class CdGDSObj;

		/// Constructor
		CdObjAttr(CdGDSObj &vOwner);
		/// Destructor
		virtual ~CdObjAttr();

    	void Assign(CdObjAttr &Source);

		CdAny &Add(const UTF16String &Name);
		int IndexName(const UTF16String &Name);
		bool HasName(const UTF16String &Name) { return IndexName(Name)>=0; }

		void Delete(const UTF16String &Name);
		void Delete(int Index);
		void Clear();

		COREARRAY_INLINE size_t Count() const { return fList.size(); }
        void Changed();

		COREARRAY_INLINE CdGDSObj &Owner() const { return fOwner; }

		CdAny & operator[](const UTF16String &Name);
		CdAny & operator[](int Index);

		COREARRAY_INLINE UTF16String &Names(int Index)
			{ return fList[Index]->name; }
		void SetName(const UTF16String &OldName, const UTF16String &NewName);
		void SetName(int Index, const UTF16String &NewName);

	protected:
		struct TdPair {
			UTF16String name;
			CdAny val;
		};

		CdGDSObj &fOwner;
		std::vector<TdPair*> fList;

		virtual void Loading(CdReader &Reader, TdVersion Version);
		virtual void Saving(CdWriter &Writer);

	private:
		std::vector<TdPair*>::iterator Find(const UTF16String &Name);
        void xValidateName(const UTF16String &name);
	};

	
	/// CoreArray GDS object
	class COREARRAY_DLL_DEFAULT CdGDSObj: public CdObjMsg
	{
	public:
		friend class CdObjAttr;
		friend class CdGDSFolder;
		friend class CdGDSVirtualFolder;
		friend class CdGDSFile;

		CdGDSObj();
		virtual ~CdGDSObj();

		virtual CdGDSObj *NewOne(void *Param = NULL);
		virtual void AssignOne(CdGDSObj &Source, void *Param = NULL);

		virtual UTF16String Name() const;
		UTF16String FullName(const UTF16String &Delimiter) const;
		UTF16String FullName(const char *Delimiter = "/") const;

		virtual void SetName(const UTF16String &NewName);

		void MoveTo(CdGDSFolder &folder);

		/// Synchronize, save data to disk
		virtual void Synchronize();

		/// Get a list of CdBlockStream owned by this object, except fGDSStream
		virtual void GetOwnBlockStream(vector<const CdBlockStream*> &Out);

		/// Get the GDS file
		CdGDSFile *GDSFile();
		COREARRAY_INLINE CdObjAttr &Attribute() { return fAttr; }
		COREARRAY_INLINE CdBlockStream *GDSStream() const { return fGDSStream; }
		COREARRAY_INLINE CdGDSFolder *Folder() const { return fFolder; }

	protected:
		CdObjAttr fAttr;
		CdGDSFolder *fFolder;
		CdBlockStream *fGDSStream;
		bool fChanged;

		virtual void LoadStruct(CdReader &Reader, TdVersion Version);
		virtual void SaveStruct(CdWriter &Writer, bool IncludeName);

		void _CheckGDSStream();
		static void _RaiseInvalidAssign(const std::string &msg);

		void SaveToBlockStream();
		virtual bool IsWithClassName() { return true; }

	#ifdef COREARRAY_CODE_USING_LOG
		virtual string LogValue();
	#endif

	private:
		static void _GDSObjInitProc(CdObjClassMgr &Sender, CdObject *dObj,
			void *Data);
	};

	/// The pointer to a GDS object
	typedef CdGDSObj *PdGDSObj;



	// =====================================================================
	// CdGDSObjPipe
	// =====================================================================

	class CdGDSObjPipe;

	/// Data pipe for compression and decompression
	class COREARRAY_DLL_DEFAULT CdPipeMgrItem: public CdAbstractItem
	{
	public:
		friend class CdStreamPipeMgr;
		friend class CdGDSObjPipe;

		/// Constructor
		CdPipeMgrItem();
		/// Destructor
		virtual ~CdPipeMgrItem();

		/// Create a new CdPipeMgrItem object
        virtual CdPipeMgrItem *NewOne() = 0;

		/// Return the name of coder stored in stream
		virtual const char *Coder() const = 0;
		/// Return the description of coder
		virtual const char *Description() const = 0;
		/// Return whether or not Mode is self
		virtual bool Equal(const char *Mode) const = 0;
		/// Return coder with parameters
		virtual string CoderParam() const = 0;

		virtual void PushReadPipe(CdBufStream &buf) = 0;
		virtual void PushWritePipe(CdBufStream &buf) = 0;
		virtual void PopPipe(CdBufStream &buf) = 0;
		virtual bool WriteMode(CdBufStream &buf) const = 0;
		virtual void ClosePipe(CdBufStream &buf) = 0;

		virtual bool GetStreamInfo(CdBufStream *BufStream) = 0;

    	void UpdateStreamSize();
		COREARRAY_INLINE SIZE64 StreamTotalIn() const { return fStreamTotalIn; }
		COREARRAY_INLINE SIZE64 StreamTotalOut() const { return fStreamTotalOut; }

		COREARRAY_INLINE CdGDSObjPipe *Owner() { return fOwner; }
		COREARRAY_INLINE TdCompressRemainder &Remainder() { return fRemainder; }

	protected:
    	CdGDSObjPipe *fOwner;
		SIZE64 fStreamTotalIn, fStreamTotalOut;
		TdCompressRemainder fRemainder;

		virtual CdPipeMgrItem *Match(const char *Mode) const = 0;
		virtual void UpdateStreamInfo(CdStream &Stream) = 0;
		virtual void LoadStream(CdReader &Reader, TdVersion Version);
		virtual void SaveStream(CdWriter &Writer);

		static bool EqualText(const char *s1, const char *s2);
	};

	/// Data pipe for compression and decompression
	class COREARRAY_DLL_DEFAULT CdPipeMgrItem2: public CdPipeMgrItem
	{
	public:
		CdPipeMgrItem2();

		/// Return whether or not Mode is self
		virtual bool Equal(const char *Mode) const;
		/// Return coder with parameters
		virtual string CoderParam() const;

	protected:
		int fCoderIndex;
		int fParamIndex;

		void ParseMode(const char *Mode, int &IdxCoder, int &IdxParam) const;
		virtual const char **CoderList() const = 0;
		virtual const char **ParamList() const = 0;
	};


	/// The manager of stream pipes
	class COREARRAY_DLL_DEFAULT CdStreamPipeMgr: public CdAbstractManager
	{
	public:
		CdStreamPipeMgr();
		~CdStreamPipeMgr();

		void Register(CdPipeMgrItem *vNewPipe);
		CdPipeMgrItem *Match(CdGDSObjPipe &Obj, const char *Mode);

		COREARRAY_INLINE const std::vector<CdPipeMgrItem*> &RegList() const
			{ return fRegList; }

	protected:
		std::vector<CdPipeMgrItem*> fRegList;
	};

	COREARRAY_DLL_DEFAULT extern CdStreamPipeMgr dStreamPipeMgr;


	/// CoreArray GDS object with a pipe object (e.g., compression/decompression)
	class COREARRAY_DLL_DEFAULT CdGDSObjPipe: public CdGDSObj
	{
	public:
		friend class CdPipeMgrItem;

		CdGDSObjPipe();
		virtual ~CdGDSObjPipe();

		virtual void AssignOne(CdGDSObj &Source, void *Param=NULL);

		// get data pipe object
		COREARRAY_INLINE CdPipeMgrItem *PipeInfo() { return fPipeInfo; }

		/// Set the mode of data storage (e.g, packed mode or compression)
		virtual void SetPackedMode(const char *Mode) = 0;

	protected:
		CdPipeMgrItem *fPipeInfo;

		virtual void Loading(CdReader &Reader, TdVersion Version);
		virtual void Saving(CdWriter &Writer);
		virtual void GetPipeInfo();

		COREARRAY_INLINE bool _GetStreamPipeInfo(CdBufStream *buf, bool Close)
		{
			if (Close && buf)
				fPipeInfo->ClosePipe(*buf);
			return fPipeInfo->GetStreamInfo(buf);
		}
		COREARRAY_INLINE void _UpdateStreamPipeInfo(CdStream &Stream)
		{
			fPipeInfo->UpdateStreamInfo(Stream);
		}
	};

	/// The pointer to a GDS object
	typedef CdGDSObjPipe *PdGDSObjPipe;



	// =====================================================================
	// CdGDSObjNoCName
	// =====================================================================

	/// the GDS object without stream name
	class COREARRAY_DLL_DEFAULT CdGDSObjNoCName: public CdGDSObj
	{
	public:
		CdGDSObjNoCName(): CdGDSObj() {}

	protected:
		virtual bool IsWithClassName() { return false; }
	};


	/// A label node for CoreArray GDS format
	class COREARRAY_DLL_DEFAULT CdGDSLabel: public CdGDSObjNoCName
	{
	public:
        virtual CdGDSObj *NewOne(void *Param = NULL);
	};


    /// Abstract folder class for CoreArray GDS format
	class COREARRAY_DLL_DEFAULT CdGDSAbsFolder: public CdGDSObj
	{
	public:
		virtual CdGDSObj *AddFolder(const UTF16String &Name) = 0;
		virtual CdGDSObj *AddObj(const UTF16String &Name, CdGDSObj *val=NULL) = 0;
		virtual CdGDSObj *InsertObj(int Index, const UTF16String &Name,
			CdGDSObj *val=NULL) = 0;
		virtual void MoveTo(int Index, int NewPos) = 0;

		virtual void DeleteObj(int Index, bool force=true) = 0;
		virtual void DeleteObj(CdGDSObj *val, bool force=true) = 0;
		virtual void ClearObj(bool force=true) = 0;

		virtual CdGDSObj *ObjItem(int Index) = 0;
		virtual CdGDSObj *ObjItem(const UTF16String &Name) = 0;

		virtual CdGDSObj *ObjItemEx(int Index) = 0;
		virtual CdGDSObj *ObjItemEx(const UTF16String &Name) = 0;

		virtual CdGDSObj *Path(const UTF16String &FullName) = 0;
		virtual CdGDSObj *PathEx(const UTF16String &FullName) = 0;

		virtual int IndexObj(CdGDSObj *Obj) = 0;
		virtual bool HasChild(CdGDSObj *Obj, bool SubFolder = true) = 0;

		virtual int NodeCount() = 0;
	};


	// =====================================================================
	// CdGDSFolder
	// =====================================================================

    /// Folder class for CoreArray GDS format
	class COREARRAY_DLL_DEFAULT CdGDSFolder: public CdGDSAbsFolder
	{
	public:
		friend class CdGDSObj;
		friend class CdGDSFile;

		virtual ~CdGDSFolder();

        virtual CdGDSObj *NewOne(void *Param = NULL);
		void AssignOneEx(CdGDSFolder &Source);

		virtual CdGDSObj *AddFolder(const UTF16String &Name);
		virtual CdGDSObj *AddObj(const UTF16String &Name, CdGDSObj *val=NULL);
		virtual CdGDSObj *InsertObj(int index, const UTF16String &Name,
			CdGDSObj *val=NULL);
		virtual void MoveTo(int Index, int NewPos);

		virtual void DeleteObj(int Index, bool force=true);
		virtual void DeleteObj(CdGDSObj *val, bool force=true);
		virtual void ClearObj(bool force=true);

		virtual CdGDSObj *ObjItem(int Index);
		virtual CdGDSObj *ObjItem(const UTF16String &Name);

		virtual CdGDSObj *ObjItemEx(int Index);
		virtual CdGDSObj *ObjItemEx(const UTF16String &Name);

		virtual CdGDSObj *Path(const UTF16String &FullName);
		virtual CdGDSObj *PathEx(const UTF16String &FullName);

		virtual int IndexObj(CdGDSObj *Obj);
		virtual bool HasChild(CdGDSObj *Obj, bool SubFolder=true);

		/// the number of child nodes in the folder
		virtual int NodeCount();

		CdGDSFolder &DirItem(int Index);
		CdGDSFolder &DirItem(const UTF16String &Name);

		COREARRAY_INLINE CdGDSObj *operator[] (int Index)
			{ return ObjItem(Index); }
		COREARRAY_INLINE CdGDSObj *operator[] (const UTF16String &Name)
			{ return ObjItem(Name); }

		static void SplitPath(const UTF16String &FullName, UTF16String &Path,
        	UTF16String &Name);

	protected:
		struct TNode
		{
		public:
			enum {
				FLAG_TYPE_CLASS           = 0,
				FLAG_TYPE_LABEL           = 1,
				FLAG_TYPE_FOLDER          = 2,
				FLAG_TYPE_VIRTUAL_FOLDER  = 3,
				FLAG_TYPE_STREAM          = 4,
				FLAG_TYPE_MASK            = 0x0F
			};

			CdGDSObj *Obj;
			TdGDSBlockID StreamID;
			C_UInt32 Flag;
			UTF16String Name;
			SIZE64 _pos;

			TNode();
			bool IsFlagType(C_UInt32 val) const;
			void SetFlagType(C_UInt32 val);
		};
		std::vector<TNode> fList;

		virtual void Loading(CdReader &Reader, TdVersion Version);
		virtual void Saving(CdWriter &Writer);
		virtual bool IsWithClassName() { return false; }

		void _ClearFolder();

	private:
		bool _HasName(const UTF16String &Name);
		TNode &_NameItem(const UTF16String &Name);
		void _LoadItem(TNode &I);
		void _UpdateAll();
		std::vector<TNode>::iterator _FindObj(CdGDSObj *Obj);
	};

	/// The pointer to a GDS folder
	typedef CdGDSFolder* PdGDSFolder;


	/// GDS virtual folder linking to another GDS file
	class COREARRAY_DLL_DEFAULT CdGDSVirtualFolder: public CdGDSAbsFolder
	{
	public:
		CdGDSVirtualFolder();
		virtual ~CdGDSVirtualFolder();

		const UTF8String &LinkFileName() const
			{ return fLinkFileName; }
		void SetLinkFile(const UTF8String &FileName);

		bool IsLoaded(bool Silent);

		virtual CdGDSObj *AddFolder(const UTF16String &Name);
		virtual CdGDSObj *AddObj(const UTF16String &Name, CdGDSObj *val=NULL);
		virtual CdGDSObj *InsertObj(int index, const UTF16String &Name,
			CdGDSObj *val=NULL);
		virtual void MoveTo(int Index, int NewPos);

		virtual void DeleteObj(int Index, bool force=true);
		virtual void DeleteObj(CdGDSObj *val, bool force=true);
		virtual void ClearObj(bool force=true);

		virtual CdGDSObj *ObjItem(int Index);
		virtual CdGDSObj *ObjItem(const UTF16String &Name);

		virtual CdGDSObj *ObjItemEx(int Index);
		virtual CdGDSObj *ObjItemEx(const UTF16String &Name);

		virtual CdGDSObj *Path(const UTF16String &FullName);
		virtual CdGDSObj *PathEx(const UTF16String &FullName);

		virtual int IndexObj(CdGDSObj *Obj);
		virtual bool HasChild(CdGDSObj *Obj, bool SubFolder=true);

		virtual int NodeCount();

		virtual void Loading(CdReader &Reader, TdVersion Version);
		virtual void Saving(CdWriter &Writer);
		virtual bool IsWithClassName() { return false; }
		virtual void Synchronize();

		const string &ErrMsg() const { return fErrMsg; }

	protected:
		UTF8String fLinkFileName;
		CdGDSFile *fLinkFile;
		bool fHasTried;
		string fErrMsg;

	private:
		void _CheckLinked();
	};


	/// Stream container for CoreArray GDS format
	class COREARRAY_DLL_DEFAULT CdGDSStreamContainer: public CdGDSObjPipe
	{
	public:
		CdGDSStreamContainer();
		virtual ~CdGDSStreamContainer();

		/// Return a string specifying the class name in stream
		virtual const char *dName();
		/// Return a string specifying the class name
		virtual const char *dTraitName();

    	/// new a CdArray<T> object
		virtual CdGDSObj *NewOne(void *Param = NULL);
		/// Assignment
		virtual void AssignOne(CdGDSObj &Source, void *Param = NULL);

		/// Get a list of CdBlockStream owned by this object, except fGDSStream
		virtual void GetOwnBlockStream(vector<const CdBlockStream*> &Out);

		void CopyFrom(CdBufStream &Source, SIZE64 Count=-1);
		void CopyFrom(CdStream &Source, SIZE64 Count=-1);

		void CopyTo(CdBufStream &Dest, SIZE64 Count=-1);
		void CopyTo(CdStream &Dest, SIZE64 Count=-1);

		SIZE64 GetSize();
		COREARRAY_INLINE CdBufStream *BufStream() { return fBufStream; }

		virtual void SetPackedMode(const char *Mode);
		virtual void CloseWriter();

	protected:
		CdBufStream *fBufStream;
		CdBlockStream *vAllocStream;
		bool fNeedUpdate;
		TdGDSBlockID vAllocID;
		SIZE64 vAlloc_Ptr;

		virtual void Loading(CdReader &Reader, TdVersion Version);
		virtual void Saving(CdWriter &Writer);
		virtual bool IsWithClassName() { return false; }
	};

	/// The pointer to a stream container
	typedef CdGDSStreamContainer *PdGDSStreamContainer;


	/// The root of a GDS file
	class COREARRAY_DLL_DEFAULT CdGDSRoot: public CdGDSFolder
	{
	public:
		friend class CdGDSVirtualFolder;

		CdGDSRoot();

		virtual UTF16String Name() const;
		virtual void SetName(const UTF16String &NewName);

	protected:
		CdGDSVirtualFolder *fVFolder;
	};



	// =====================================================================
	// CdGDSUnknown
	// =====================================================================

	// GDS node indicating unknown states
	class COREARRAY_DLL_DEFAULT CdGDSUnknown: public CdGDSObjNoCName
	{
	public:
		CdGDSUnknown();
		virtual void SaveStruct(CdWriter &Writer, bool IncludeName);
	};



	// =====================================================================
	// CdGDSFile
	// =====================================================================

	/// CoreArray GDS File
	class COREARRAY_DLL_DEFAULT CdGDSFile: protected CdBlockCollection
	{
	public:
		friend class CdGDSVirtualFolder;

		enum TdOpenMode { dmCreate, dmOpenRead, dmOpenReadWrite };

		CdGDSFile();
		CdGDSFile(const UTF8String &fn, TdOpenMode Mode);
		CdGDSFile(const char *fn, TdOpenMode Mode);
		virtual ~CdGDSFile();

		void LoadFile(const UTF8String &fn, bool ReadOnly=true);
		void LoadFile(const char *fn, bool ReadOnly=true);
		void LoadFileFork(const char *fn, bool ReadOnly=true);

		void SaveAsFile(const UTF8String &fn);
		void SaveAsFile(const char *fn);

		void DuplicateFile(const UTF8String &fn, bool deep);
		void DuplicateFile(const char *fn, bool deep);

		void SyncFile();
		void CloseFile();

		/// Clean up all fragments
		void TidyUp(bool deep);

		bool Modified();

		/// Return file size of the CdGDSFile object
		SIZE64 GetFileSize();

		int GetNumOfFragment();

		bool IfSupportForking();
		TProcessID GetProcessID();
		void SetProcessID();

		/// Return the file name of the CdGDSFile object
		COREARRAY_INLINE UTF8String &FileName() { return fFileName; }
		COREARRAY_INLINE CdGDSFolder &Root() { return fRoot; }
		COREARRAY_INLINE bool ReadOnly() const { return fReadOnly; }
		COREARRAY_INLINE CdLogRecord &Log() { return *fLog; }
		COREARRAY_INLINE TdVersion Version() const { return fVersion; }

		static const char *GDSFilePrefix();

	protected:
		TdVersion fVersion;
		CdGDSRoot fRoot;
		bool fReadOnly;
		UTF8String fFileName;

		void LoadStream(CdStream* Stream, bool ReadOnly = true);
		void SaveStream(CdStream* Stream);

	private:
        CdLogRecord *fLog;
        TProcessID fprocess_id;

		void _Init();
		bool _HaveModify(CdGDSFolder *folder);
	};

	/// The pointer to a CoreArray GDS File
	typedef CdGDSFile* PdGDSFile;



	/// Exception for CdGDSObj
	class COREARRAY_DLL_EXPORT ErrGDSObj: public ErrObject
	{
	public:
		ErrGDSObj(): ErrObject()
			{ }
		ErrGDSObj(const std::string &msg): ErrObject()
			{ fMessage = msg; }
		ErrGDSObj(const char *fmt, ...): ErrObject()
			{ _COREARRAY_ERRMACRO_(fmt); }
	};

	/// Exception for stream container
	class COREARRAY_DLL_EXPORT ErrGDSStreamContainer: public ErrObject
	{
	public:
		ErrGDSStreamContainer(): ErrObject()
			{ }
		ErrGDSStreamContainer(const std::string &msg): ErrObject()
			{ fMessage = msg; }
		ErrGDSStreamContainer(const char *fmt, ...): ErrObject()
			{ _COREARRAY_ERRMACRO_(fmt); }
	};

	/// Exception for GDS file
	class COREARRAY_DLL_EXPORT ErrGDSFile: public ErrObject
	{
	public:
		ErrGDSFile(): ErrObject()
			{ }
		ErrGDSFile(const std::string &msg): ErrObject()
			{ fMessage = msg; }
		ErrGDSFile(const char *fmt, ...): ErrObject()
			{ _COREARRAY_ERRMACRO_(fmt); }
	};
}

#endif /* _HEADER_COREARRAY_FILE_ */
