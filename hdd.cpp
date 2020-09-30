#include "hdd.h"
#include "strcrypt.h"

int ReadPhysicalDriveInNTWithAdminRights (char* serial)
{
   int done = FALSE;
   int drive = 0;

   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

      char driveName [256];

      wsprintfA (driveName, F("\\\\.\\PhysicalDrive%d"), drive);

      hPhysicalDriveIOCTL = CreateFileA (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_READ | FILE_SHARE_WRITE , NULL,
                               OPEN_EXISTING, 0, NULL);

      if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
	  {
			GETVERSIONOUTPARAMS VersionParams = {0};
			DWORD cbBytesReturned = 0;

         DeviceIoControl (hPhysicalDriveIOCTL, DFP_GET_VERSION,
                   NULL, 
                   0,
                   &VersionParams,
                   sizeof(VersionParams),
                   &cbBytesReturned, NULL);

         if (VersionParams.bIDEDeviceMap > 0)
         {
            BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
            SENDCMDINPARAMS  scip;

            bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? \
                      IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

            memset (&scip, 0, sizeof(scip));
            memset (IdOutCmd, 0, sizeof(IdOutCmd));

            if ( DoIDENTIFY (hPhysicalDriveIOCTL, 
                       &scip, 
                       (PSENDCMDOUTPARAMS)&IdOutCmd, 
                       (BYTE) bIDCmd,
                       (BYTE) drive,
                       &cbBytesReturned))
            {
               DWORD diskdata [256];
               int ijk = 0;
               USHORT *pIdSector = (USHORT *)
                             ((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;

               for (ijk = 0; ijk < 256; ijk++)
                  diskdata [ijk] = pIdSector [ijk];

			   char serialNumber[255];

               ConvertToString (diskdata, 10, 19, serialNumber);

			 if(strlen(serialNumber)>0 && strlen(serialNumber)<128)
			 {
				 strtrim(serial,serialNumber,40);
				 CloseHandle (hPhysicalDriveIOCTL);
				 return 1;
			 }
            }
	    }

         CloseHandle (hPhysicalDriveIOCTL);
      }
   }

   return 0;
}

int ReadPhysicalDriveInNTUsingSmart (char* serial)
{
   int done = FALSE;
   int drive = 0;

   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;
      char driveName [256];

      wsprintfA (driveName, F("\\\\.\\PhysicalDrive%d"), drive);

      hPhysicalDriveIOCTL = CreateFileA (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
							   NULL, OPEN_EXISTING, 0, NULL);

      if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
      {
		  GETVERSIONINPARAMS GetVersionParams = {0};
         DWORD cbBytesReturned = 0;

         if ( DeviceIoControl (hPhysicalDriveIOCTL, SMART_GET_VERSION,
                   NULL, 
                   0,
     			   &GetVersionParams, sizeof (GETVERSIONINPARAMS),
				   &cbBytesReturned, NULL) )
         {
			ULONG CommandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
        	PSENDCMDINPARAMS Command = (PSENDCMDINPARAMS) malloc (CommandSize);

			Command -> irDriveRegs.bCommandReg = 0xEC;
			DWORD BytesReturned = 0;
	        if ( DeviceIoControl (hPhysicalDriveIOCTL, 
				                    SMART_RCV_DRIVE_DATA, Command, sizeof(SENDCMDINPARAMS),
									Command, CommandSize,
									&BytesReturned, NULL) )
			{
        	       // Print the IDENTIFY data
                DWORD diskdata [256];
                USHORT *pIdSector = (USHORT *)
                             (PIDENTIFY_DATA) ((PSENDCMDOUTPARAMS) Command) -> bBuffer;

                for (int ijk = 0; ijk < 256; ijk++)
                   diskdata [ijk] = pIdSector [ijk];

                char serialNumber[255];

               ConvertToString (diskdata, 10, 19, serialNumber);

			 if(strlen(serialNumber)>0 && strlen(serialNumber)<128)
			 {
				 strtrim(serial,serialNumber,40);
				 CloseHandle (hPhysicalDriveIOCTL);
				 free (Command);
				 return 1;
			 }
			}
	           // Done
            CloseHandle (hPhysicalDriveIOCTL);
			free (Command);
		 }
      }
   }

   return 0;
}


BOOL DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
                 PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
                 PDWORD lpcbBytesReturned)
{
      // Set up data structures for IDENTIFY command.
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;
   pSCIP -> irDriveRegs.bFeaturesReg = 0;
   pSCIP -> irDriveRegs.bSectorCountReg = 1;
   //pSCIP -> irDriveRegs.bSectorNumberReg = 1;
   pSCIP -> irDriveRegs.bCylLowReg = 0;
   pSCIP -> irDriveRegs.bCylHighReg = 0;

      // Compute the drive number.
   pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

      // The command can either be IDE identify or ATAPI identify.
   pSCIP -> irDriveRegs.bCommandReg = bIDCmd;
   pSCIP -> bDriveNumber = bDriveNum;
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;

   return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
               (LPVOID) pSCIP,
               sizeof(SENDCMDINPARAMS) - 1,
               (LPVOID) pSCOP,
               sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               lpcbBytesReturned, NULL) );
}

int ReadPhysicalDriveInNTWithZeroRights (char* serial)
{
   int done = FALSE;
   int drive = 0;

   for (drive = 0; drive < MAX_IDE_DRIVES; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      char driveName [256];

      wsprintfA (driveName, F("\\\\.\\PhysicalDrive%d"), drive);

         //  Windows NT, Windows 2000, Windows XP - admin rights not required
      hPhysicalDriveIOCTL = CreateFileA (driveName, 0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);
      if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
      {
		 STORAGE_PROPERTY_QUERY query;
         DWORD cbBytesReturned = 0;
		 char buffer [10000];

         memset ((void *) & query, 0, sizeof (query));
		 query.PropertyId = StorageDeviceProperty;
		 query.QueryType = PropertyStandardQuery;

		 memset (buffer, 0, sizeof (buffer));

         if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
                   & query,
                   sizeof (query),
				   & buffer,
				   sizeof (buffer),
                   & cbBytesReturned, NULL) )
         {         
			 STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *) & buffer;
			 char serialNumber [1000];

	         flipAndCodeBytes (buffer,
			                   descrip -> SerialNumberOffset,
			                   1, serialNumber );
			 if(strlen(serialNumber)>0 && strlen(serialNumber)<128)
			 {
				 strtrim(serial,serialNumber,40);
				 CloseHandle (hPhysicalDriveIOCTL);
				 return 1;
			 }

	         memset (buffer, 0, sizeof(buffer));	     
         }

         CloseHandle (hPhysicalDriveIOCTL);
      }
   }

   return 0;
}

int ReadIdeDriveAsScsiDriveInNT (char* serial)
{
   int done = FALSE;
   int controller = 0;

   for (controller = 0; controller < 16; controller++)
   {
      HANDLE hScsiDriveIOCTL = 0;
      char   driveName [256];

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.
      wsprintfA (driveName, F("\\\\.\\Scsi%d:"), controller);

         //  Windows NT, Windows 2000, any rights should do
      hScsiDriveIOCTL = CreateFileA (driveName,
                               GENERIC_READ | GENERIC_WRITE, 
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);

      if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
      {
         int drive = 0;

         for (drive = 0; drive < 2; drive++)
         {
            char buffer [sizeof (SRB_IO_CONTROL) + SENDIDLENGTH];
            SRB_IO_CONTROL *p = (SRB_IO_CONTROL *) buffer;
            SENDCMDINPARAMS *pin =
                   (SENDCMDINPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
            DWORD dummy;
   
            memset (buffer, 0, sizeof (buffer));
            p -> HeaderLength = sizeof (SRB_IO_CONTROL);
            p -> Timeout = 10000;
            p -> Length = SENDIDLENGTH;
            p -> ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
            strncpy ((char *) p -> Signature, F("SCSIDISK"), 8);
  
            pin -> irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
            pin -> bDriveNumber = drive;

            if (DeviceIoControl (hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, 
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) +
                                         sizeof (SENDCMDINPARAMS) - 1,
                                 buffer,
                                 sizeof (SRB_IO_CONTROL) + SENDIDLENGTH,
                                 &dummy, NULL))
            {
               SENDCMDOUTPARAMS *pOut =
                    (SENDCMDOUTPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
               IDSECTOR *pId = (IDSECTOR *) (pOut -> bBuffer);
               if (pId -> sModelNumber [0])
               {
                  DWORD diskdata [256];
                  int ijk = 0;
                  USHORT *pIdSector = (USHORT *) pId;
          
                  for (ijk = 0; ijk < 256; ijk++)
                     diskdata [ijk] = pIdSector [ijk];

				  char serialNumber[255];

				 ConvertToString (diskdata, 10, 19, serialNumber);

                 if(strlen(serialNumber)>0 && strlen(serialNumber)<128)
				 {
					 strtrim(serial,serialNumber,40);
					 CloseHandle (hScsiDriveIOCTL);
					 return 1;
				 }
               }
            }
         }
         CloseHandle (hScsiDriveIOCTL);
      }
   }

   return 0;
}


char * flipAndCodeBytes (const char * str,
			 int pos,
			 int flip,
			 char * buf)
{
   int i;
   int j = 0;
   int k = 0;

   buf [0] = '\0';
   if (pos <= 0)
      return buf;

   if ( ! j)
   {
      char p = 0;

      // First try to gather all characters representing hex digits only.
      j = 1;
      k = 0;
      buf[k] = 0;
      for (i = pos; j && str[i] != '\0'; ++i)
      {
	 char c = (char)CharLowerA((LPSTR)str[i]);

	 if (isspace(c))
	    c = '0';

	 ++p;
	 buf[k] <<= 4;

	 if (c >= '0' && c <= '9')
	    buf[k] |= (unsigned char) (c - '0');
	 else if (c >= 'a' && c <= 'f')
	    buf[k] |= (unsigned char) (c - 'a' + 10);
	 else
	 {
	    j = 0;
	    break;
	 }

	 if (p == 2)
	 {
	    if (buf[k] != '\0' && ! isprint(buf[k]))
	    {
	       j = 0;
	       break;
	    }
	    ++k;
	    p = 0;
	    buf[k] = 0;
	 }

      }
   }

   if ( ! j)
   {
      // There are non-digit characters, gather them as is.
      j = 1;
      k = 0;
      for (i = pos; j && str[i] != '\0'; ++i)
      {
	     char c = str[i];

	     if ( ! isprint(c))
	     {
	        j = 0;
	        break;
	     }

	     buf[k++] = c;
      }
   }

   if ( ! j)
   {
      // The characters are not there or are not printable.
      k = 0;
   }

   buf[k] = '\0';

   if (flip)
      // Flip adjacent characters
      for (j = 0; j < k; j += 2)
      {
	     char t = buf[j];
	     buf[j] = buf[j + 1];
	     buf[j + 1] = t;
      }

   // Trim any beginning and end space
   i = j = -1;
   for (k = 0; buf[k] != '\0'; ++k)
   {
      if (! isspace(buf[k]))
      {
	     if (i < 0)
	        i = k;
	     j = k;
      }
   }

   if ((i >= 0) && (j >= 0))
   {
      for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
         buf[k - i] = buf[k];
      buf[k - i] = '\0';
   }

   return buf;
}



char *ConvertToString (DWORD diskdata [256],
		       int firstIndex,
		       int lastIndex,
		       char* buf)
{
   int index = 0;
   int position = 0;

      //  each integer has two characters stored in it backwards
   for (index = firstIndex; index <= lastIndex; index++)
   {
         //  get high byte for 1st character
      buf [position++] = (char) (diskdata [index] / 256);

         //  get low byte for 2nd character
      buf [position++] = (char) (diskdata [index] % 256);
   }

      //  end the string 
   buf[position] = '\0';

      //  cut off the trailing blanks
   for (index = position - 1; index > 0 && isspace(buf [index]); index--)
      buf [index] = '\0';

   return buf;
}


int getIDs(char* serial)
{
	return (int)(ReadPhysicalDriveInNTWithAdminRights(serial) || ReadPhysicalDriveInNTWithZeroRights(serial) \
		|| ReadPhysicalDriveInNTUsingSmart(serial) || ReadIdeDriveAsScsiDriveInNT(serial));
}