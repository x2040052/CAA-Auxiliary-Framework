//===================================================================
// COPYRIGHT Tony 2018/05/14
//===================================================================
// CsvReader.cpp
// Header definition of class JNCCsvReader
//===================================================================
//
// Usage notes:
//
//===================================================================
//  2018/05/14 Creation: Code generated by the 3DS wizard
//===================================================================

#include "CsvReader.h"

//-----------------------------------------------------------------------------
// CsvReader : constructor
//-----------------------------------------------------------------------------
CsvReader::CsvReader()
{
	
}

//-----------------------------------------------------------------------------
// CsvReader : destructor
//-----------------------------------------------------------------------------

CsvReader::~CsvReader()
{
	
}

CsvReader* CsvReader::GetCsvReader()
{
	static CsvReader singleReader;
	return &singleReader;
}

void CsvReader::ClearCache()
{
	GetCsvReader()->m_Recorder.clear();
	return;
}

CATBoolean CsvReader::IsCsvFile( LPCSTR iFileName )
{
	if (NULL == iFileName || 4 > strlen(iFileName))
		return FALSE;

	string _strFileName = string(iFileName);
	size_t _iLen = _strFileName.length();
	string _strSuff(_strFileName.substr(_iLen - 4, 4));

	transform(_strSuff.begin(),_strSuff.end(),_strSuff.begin(),tolower);

	return (CATBoolean(0 == _strSuff.compare(".csv")));
}

HRESULT CsvReader::ReadCsvData( LPCSTR iFileName, CsvData*& oData )
{
	CATUnicodeString catFileName = iFileName;

	if (!IsCsvFile(iFileName))
		return E_INVALIDARG;

	if (NULL != oData) // 传入一个空指针
		return E_UNEXPECTED;

	auto it_Recorder = m_Recorder.find(catFileName);
	if (it_Recorder != m_Recorder.end())
	{
		oData = &(it_Recorder->second);
		return S_OK;
	}

	ifstream _streamFromFile(iFileName);
	if (NULL == _streamFromFile)
		return E_ACCESSDENIED;

	string _strRow = "";
	CATUnicodeString _catstrRow = "";
	vector<CATUnicodeString> ColumnHead;
	vector<CATUnicodeString>::iterator it_ColumnHead;


	oData = new CsvData;

	int ii = 0;

	while (getline(_streamFromFile, _strRow))
	{
		LPCSTR _pcSrc = _strRow.c_str();

		map<CATUnicodeString, CATUnicodeString> _mapForOneRow;
		if (0 != ii)
			it_ColumnHead = ColumnHead.begin();

		//CATUnicodeString RowKey = ""; // 取消行键值，改为Vector
		//int jj = 1;
		while ( *_pcSrc != '\0')
		{
			string _strEle("");
			if (*_pcSrc == '"')
			{
				_pcSrc ++;
				while (*_pcSrc != '\0')
				{
					if (*_pcSrc == '"')
					{
						_pcSrc ++; // 一对 " 保留一个
						if (*_pcSrc != '"')
						{
							_pcSrc ++;
							break;
						}
					}
					_strEle.push_back(*_pcSrc++);
				}
			}
			else
			{
				while ( *_pcSrc != '\0' && *_pcSrc != ',')
					_strEle.push_back(*_pcSrc++);

				if (* _pcSrc != '\0')
					_pcSrc++;
			}

			//if (1 == jj)
			//{
			//	RowKey = _strEle.c_str();
			//	jj ++;
			//}

			_catstrRow = _strEle.c_str();
			if (0 == ii)
			{
				ColumnHead.push_back(_catstrRow);
			}
			else
			{
				if (it_ColumnHead != ColumnHead.end())
				{
					_mapForOneRow[*it_ColumnHead] = _catstrRow;
					it_ColumnHead++;
				}
			}
		}

		if (0 < ii)
		{
			//RowKey.BuildFromNum(ii);
			oData->push_back(_mapForOneRow);
		}

		_strRow.assign("");

		ii++;
	}

	HRESULT rc = E_FAIL;
	if (1 < ii)
	{	
		m_Recorder[catFileName] = *oData;
	}

	delete oData;

	it_Recorder = m_Recorder.find(catFileName);
	if (it_Recorder != m_Recorder.end())
	{
		oData = &(it_Recorder->second);
		rc = S_OK;
	}

	return rc;
}

HRESULT CsvReader::ReadCsvRow( CATUnicodeString iFileName, CATUnicodeString iColumnHead, CATUnicodeString iRowKey, CATListOfCATUnicodeString& oRowValueList )
{
	LPCSTR _FileName = iFileName.ConvertToChar();
	CsvData *_pFileData = NULL;
	HRESULT rc = E_FAIL;

	oRowValueList.RemoveAll();
	rc = this->ReadCsvData(_FileName, _pFileData);

	if (SUCCEEDED(rc) && NULL != _pFileData)
	{
		auto it_Row = _pFileData->begin();
		while (it_Row != _pFileData->end())
		{
			map<CATUnicodeString,CATUnicodeString> * pRowMap = NULL;
			pRowMap = &(*it_Row);
			auto it_Column = pRowMap->find(iColumnHead);
			if (it_Column != pRowMap->end() && iRowKey == it_Column->second)
			{
				rc = S_OK;
				it_Column = pRowMap->begin();
				while (it_Column != pRowMap->end())
				{
					oRowValueList.Append(it_Column->second);
					it_Column ++;
				}	
				break;
			}
			it_Row++ ;
		}
	}

	return rc;
}

HRESULT CsvReader::ReadCsvColumn( CATUnicodeString iFileName, CATUnicodeString iColumHead, CATListOfCATUnicodeString& oColumnValueList )
{
	LPCSTR _FileName = iFileName.ConvertToChar();
	CsvData *_pFileData = NULL;
	HRESULT rc = E_FAIL;

	rc = this->ReadCsvData(_FileName, _pFileData);

	oColumnValueList.RemoveAll();
	if (SUCCEEDED(rc) && NULL != _pFileData)
	{
		auto it_Row = _pFileData->begin();
		while (it_Row != _pFileData->end())
		{
			map<CATUnicodeString,CATUnicodeString> * pRowMap = NULL;
			pRowMap = &(*it_Row);
			auto it_Column = pRowMap->find(iColumHead);
			if (it_Column != pRowMap->end() && (it_Column->second != NULL))
				oColumnValueList.Append(it_Column->second);

			it_Row++ ;
		}
	}

	return rc;
}

HRESULT CsvReader::ReadCsvColumnWithUniqueValue( CATUnicodeString iFileName, CATUnicodeString iColumHead, CATListOfCATUnicodeString& oColumnValueList )
{
	LPCSTR _FileName = iFileName.ConvertToChar();
	CsvData *_pFileData = NULL;
	HRESULT rc = E_FAIL;

	rc = this->ReadCsvData(_FileName, _pFileData);

	oColumnValueList.RemoveAll();
	if (SUCCEEDED(rc) && NULL != _pFileData)
	{
		auto it_Row = _pFileData->begin();
		while (it_Row != _pFileData->end())
		{
			map<CATUnicodeString,CATUnicodeString> * pRowMap = NULL;
			pRowMap = &(*it_Row);
			auto it_Column = pRowMap->find(iColumHead);
			//cout<<"bump locate : "<<oColumnValueList.Locate(it_Column->second)<<" and value is "<<it_Column->second.ConvertToChar()<<endl;
			if (it_Column != pRowMap->end() && (it_Column->second != NULL) && (0 == oColumnValueList.Locate(it_Column->second)))
				oColumnValueList.Append(it_Column->second);

			it_Row++ ;
		}
	}

	return rc;
}

CATUnicodeString CsvReader::ReadCsvCell( CATUnicodeString iFileName, CATUnicodeString iRowKeyColumnHead, CATUnicodeString iRowKey, CATUnicodeString iColumKey )
{
	HRESULT rc = E_FAIL;
	CsvData *_pFileData = NULL;
	rc = this->ReadCsvData(iFileName, _pFileData);

	if (SUCCEEDED(rc) && NULL != _pFileData)
	{
		auto it_Row = _pFileData->begin();
		while (it_Row != _pFileData->end())
		{
			map<CATUnicodeString,CATUnicodeString> * pRowMap = NULL;
			pRowMap = &(*it_Row);
			auto it_Column = pRowMap->find(iRowKeyColumnHead);
			if (it_Column != pRowMap->end() && iRowKey == it_Column->second)
			{
				it_Column = pRowMap->find(iColumKey);
				if (it_Column != pRowMap->end())
				{
					return it_Column->second;
				}	
			}
			it_Row++ ;
		}
	}

	CATUnicodeString NullString = "";
	return NullString;
}

bool CsvReader::CheckSpecifyContent( CATUnicodeString iFileName, CATListOfCATUnicodeString& iHeadList, CATListOfCATUnicodeString& iKeyList )
{
	bool rlt = false;

	if (0 == iHeadList.Size() || iHeadList.Size() != iKeyList.Size())
		return rlt;

	HRESULT rc = E_FAIL;
	CsvData *_pFileData = NULL;
	rc = this->ReadCsvData(iFileName, _pFileData);

	if (SUCCEEDED(rc) && NULL != _pFileData)
	{
		auto it_Row = _pFileData->begin();
		bool eachRow = true;
		while (it_Row != _pFileData->end())
		{
			eachRow = true;
			map<CATUnicodeString,CATUnicodeString> * pRowMap = NULL;
			pRowMap = &(*it_Row);
			for (int i = 1 ; i <= iHeadList.Size(); i ++)
			{
				auto it_Column = pRowMap->find(iHeadList[i]);
				if (it_Column != pRowMap->end())
				{
					if (iKeyList[i] != it_Column->second)
						eachRow = false;	
				}
				else
					eachRow = false;
			}

			if (eachRow)
			{
				rlt = true;
				break;
			}
			it_Row++ ;
		}
	}

	return rlt;
}

FileState CsvReader::CheckCsvFile( CATUnicodeString iFileName )
{
	if (!IsCsvFile(iFileName.ConvertToChar()))
		return FileState::InvalidFileName;

	ifstream _streamFromFile(iFileName.ConvertToChar());
	if (NULL == _streamFromFile)
		return FileState::ReadingError;

	string _strRow = "";
	if ( !getline(_streamFromFile, _strRow)) // 首行
		return FileState::FormatError;

	_strRow.assign("");
	if ( !getline(_streamFromFile, _strRow)) // 至少两行
		return FileState::FormatError;

	return FileState::ReadOK;
}

HRESULT CsvReader::ReadCsvHeads( LPCSTR iFileName,CATListOfCATUnicodeString& oHeadList )
{
	CATUnicodeString catFileName = iFileName;
	oHeadList.RemoveAll();

	if (!IsCsvFile(iFileName))
		return E_INVALIDARG;

	ifstream _streamFromFile(iFileName);
	if (NULL == _streamFromFile)
		return E_ACCESSDENIED;

	string _strRow = "";
	CATUnicodeString _catstrRow = "";

	int ii = 0;

	if (getline(_streamFromFile, _strRow))
	{
		LPCSTR _pcSrc = _strRow.c_str();

		while ( *_pcSrc != '\0')
		{
			string _strEle("");
			if (*_pcSrc == '"')
			{
				_pcSrc ++;
				while (*_pcSrc != '\0')
				{
					if (*_pcSrc == '"')
					{
						_pcSrc ++; // 一对 " 保留一个
						if (*_pcSrc != '"')
						{
							_pcSrc ++;
							break;
						}
					}
					_strEle.push_back(*_pcSrc++);
				}
			}
			else
			{
				while ( *_pcSrc != '\0' && *_pcSrc != ',')
					_strEle.push_back(*_pcSrc++);

				if (* _pcSrc != '\0')
					_pcSrc++;
			}

			_catstrRow = _strEle.c_str();

			oHeadList.Append(_catstrRow);
		}
	}

	if (0 < oHeadList.Size())
		return S_OK;
	else
		return E_UNEXPECTED;

}



