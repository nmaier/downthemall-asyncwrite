/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is DownThemAll! AsyncFileOutputStream.
 *
 * The Initial Developer of the Original Code is
 * Nils Maier.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Nils Maier <maierman@web.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "AsyncFileOutputStream.h"
#include "delayed_stream.h"

#include "nsILocalFile.h"

#include "nsAlgorithm.h"
#include "nsStringAPI.h"

AsyncFileOutputStream::AsyncFileOutputStream()
  : mOffset(0),
  mMaxOffset(0),
  mStream(0),
  mClosed(false)
{
}

AsyncFileOutputStream::~AsyncFileOutputStream()
{
  Close();
}

NS_IMPL_THREADSAFE_ISUPPORTS4(AsyncFileOutputStream, dtaIAsyncFileOutputStream, nsIOutputStream, nsIFileOutputStream, nsISeekableStream)

/* nsIOutputStream */
NS_IMETHODIMP
AsyncFileOutputStream::Close()
{
  NS_ENSURE_TRUE(mStream, NS_ERROR_NOT_INITIALIZED);
  NS_ENSURE_TRUE(!mClosed, NS_BASE_STREAM_CLOSED);

  delayed_stream_close(mStream);
  mStream = 0;
  mClosed = true;
  return NS_OK;
}

NS_IMETHODIMP
AsyncFileOutputStream::Flush()
{
  NS_ENSURE_TRUE(mStream, NS_ERROR_NOT_INITIALIZED);
  NS_ENSURE_TRUE(!mClosed, NS_BASE_STREAM_CLOSED);
  delayed_stream_flush(mStream);
  return NS_OK;
}

NS_IMETHODIMP
AsyncFileOutputStream::Write(const char *buf, PRUint32 count, PRUint32 *result)
{
  NS_ENSURE_TRUE(mStream, NS_ERROR_NOT_INITIALIZED);
  NS_ENSURE_TRUE(!mClosed, NS_BASE_STREAM_CLOSED);
  NS_ENSURE_ARG_POINTER(buf);
  NS_ENSURE_TRUE(count, NS_ERROR_INVALID_ARG);
  NS_ENSURE_ARG_POINTER(result);

  if (!delayed_stream_write(mStream, mOffset, buf, count)) {
    Close();
    return NS_ERROR_FILE_ACCESS_DENIED;
  }
  *result = count;
  mOffset += count;
  mMaxOffset = NS_MAX(mMaxOffset, mOffset);

  return NS_OK;
}

NS_IMETHODIMP
AsyncFileOutputStream::WriteFrom(nsIInputStream *inStr, PRUint32 count, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
AsyncFileOutputStream::WriteSegments(nsReadSegmentFun reader, void * closure, PRUint32 count, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
AsyncFileOutputStream::IsNonBlocking(PRBool *aNonBlocking)
{
  NS_ENSURE_ARG_POINTER(aNonBlocking);
    *aNonBlocking = PR_FALSE;
    return NS_OK;
}

/* nsIFileOutputStream */
NS_IMETHODIMP
AsyncFileOutputStream::Init(nsIFile* aFile, PRInt32 aIOFlags, PRInt32 aPerm,
                        PRInt32 aBehaviorFlags)
{
    NS_ENSURE_TRUE(!mStream, NS_ERROR_ALREADY_INITIALIZED);
    NS_ENSURE_TRUE(!mClosed, NS_ERROR_ALREADY_INITIALIZED);
  NS_ENSURE_ARG_POINTER(aFile);

  nsresult rv;

  nsCOMPtr<nsILocalFile> localFile = do_QueryInterface(aFile, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  nsString name;
  rv = localFile->GetTarget(name);
  NS_ENSURE_SUCCESS(rv, rv);

  mStream = delayed_stream_open(name.get());
  if (!mStream) {
    return NS_ERROR_FILE_ACCESS_DENIED;
  }
  return NS_OK;
}


/* nsISeekableStream */
NS_IMETHODIMP
AsyncFileOutputStream::Seek(PRInt32 whence, PRInt64 offset)
{
  switch (whence) {
    case NS_SEEK_SET:
      mOffset = NS_MAX(offset, (__int64)0);
      break;
    case NS_SEEK_CUR:
      mOffset = NS_MAX(mOffset + offset, (__int64)0);
      break;
    case NS_SEEK_END:
      mOffset = NS_MAX(mMaxOffset + offset, (__int64)0);
      break;

    default:
      NS_NOTREACHED("whence");
      return NS_ERROR_INVALID_ARG;
  }
  mMaxOffset = NS_MAX(mOffset, (__int64)0);
  return NS_OK;
}

NS_IMETHODIMP
AsyncFileOutputStream::Tell(PRInt64 *result)
{
  NS_ENSURE_ARG_POINTER(result);
  *result = mOffset;
  return NS_OK;
}

NS_IMETHODIMP
AsyncFileOutputStream::SetEOF()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

