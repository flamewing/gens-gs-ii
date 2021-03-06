/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromDrives.cpp: Find CD-ROM drives: Manager class.                 *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "FindCdromDrives.hpp"
#include "GensQApplication.hpp"

// C includes.
#include <stdio.h>

// LibGensCD includes.
#include "libgenscd/DiscType.h"

// Qt includes.
#include <QtCore/QMap>

// OS-specific FindCdromBase subclasses.
// TODO: Add Mac OS X, and non-UDisks classes.
#if defined(Q_OS_WIN)
#include "cdrom/FindCdromWin32.hpp"
#elif defined(QT_QTDBUS_FOUND)
#include "cdrom/FindCdromUDisks2.hpp"
#include "cdrom/FindCdromUDisks.hpp"
#endif

// UNIX fallback.
#if defined(Q_OS_UNIX)
#include "cdrom/FindCdromUnix.hpp"
#endif

namespace GensQt4 {

class FindCdromDrivesPrivate
{
	public:
		FindCdromDrivesPrivate(FindCdromDrives *q);
		~FindCdromDrivesPrivate();

	private:
		FindCdromDrives *const q_ptr;
		Q_DECLARE_PUBLIC(FindCdromDrives)
	private:
		Q_DISABLE_COPY(FindCdromDrivesPrivate)

	public:
		// List of CD-ROM device names.
		QStringList cdromDeviceNames;

		// Map of CD-ROM device names to LibGensCD::CdDrive instances.
		// NOTE: Using QMap instead of QHash because most people will
		// probably have a maximum of 2 physical drives + 1 virtual drive.
		// Reference: http://woboq.com/blog/qmap_qhash_benchmark.html
		QMap<QString, LibGensCD::CdDrive*> mapCdromDevices;

		// FindCdromBase instance.
		// TODO: Create FindCdromBase class.
		FindCdromBase *findCdromBase;

		// Clear all CD-ROM devices.
		void clearCdromDevices(void);

		/**
		 * Add a CD-ROM device.
		 * @param deviceName CD-ROM device name.
		 */
		void addCdromDevice(const QString &deviceName);

		/**
		 * Remove a CD-ROM device.
		 * @param deviceName CD-ROM device name.
		 */
		void removeCdromDevice(const QString &deviceName);

		/**
		 * Get the icon for a given drive type.
		 * @param driveType Drive type.
		 * @return Drive type icon, or invalid icon if the drive type is either DRIVE_TYPE_NONE or invalid.
		 */
		static QIcon GetDriveTypeIcon(CD_DriveType_t driveType);

		/**
		 * Get the icon for a given disc type.
		 * NOTE: The disc type MUST be a single bit! (Combinations are treated as invalid.)
		 * @param discType Disc type.
		 * @return Disc type icon, or invalid icon if the disc type is DISC_TYPE_NONE or invalid.
		 */
		static QIcon GetDiscTypeIcon(CD_DiscType_t discType);
};


/** FindCdromDrivesPrivate **/

FindCdromDrivesPrivate::FindCdromDrivesPrivate(FindCdromDrives *q)
	: q_ptr(q)
	, findCdromBase(nullptr)
{
	// Initialize the FindCdromBase class.
#if defined(Q_OS_WIN)
	findCdromBase = new FindCdromWin32(q);
#elif defined(QT_QTDBUS_FOUND)
	// Test UDisks2, then UDisks1.
	findCdromBase = new FindCdromUDisks2(q);
	if (findCdromBase && !findCdromBase->isUsable()) {
		delete findCdromBase;
		findCdromBase = nullptr;
	}

	if (!findCdromBase)
		findCdromBase = new FindCdromUDisks(q);
#else
	// TODO: Implement FindCdromBase subclass for Mac OS X.
	findCdromBase = nullptr;
#endif

	if (findCdromBase && !findCdromBase->isUsable()) {
		// FindCdromBase class isn't usable.
		delete findCdromBase;
		findCdromBase = nullptr;
	}

	// Check for fallbacks in case the primary FindCdromBase class isn't usable.
	if (!findCdromBase) {
#if defined(Q_OS_UNIX)
		// UNIX fallback.
		findCdromBase = new FindCdromUnix(q);
#endif
	}

	if (findCdromBase && !findCdromBase->isUsable()) {
		// FindCdromBase class still isn't usable.
		delete findCdromBase;
		findCdromBase = nullptr;
	}
}

FindCdromDrivesPrivate::~FindCdromDrivesPrivate()
{
	clearCdromDevices();
	delete findCdromBase;
}

/**
 * Clear all CD-ROM devices.
 */
void FindCdromDrivesPrivate::clearCdromDevices(void)
{
	// Clear the device name string list.
	Q_Q(FindCdromDrives);
	foreach (QString deviceName, cdromDeviceNames) {
		emit q->driveRemoved(deviceName);
	}
	cdromDeviceNames.clear();

	// Clear the map of LibGensCD::CdDrive instances.
	qDeleteAll(mapCdromDevices);
	mapCdromDevices.clear();
}

/**
 * Add a CD-ROM device.
 * @param deviceName CD-ROM device name.
 */
void FindCdromDrivesPrivate::addCdromDevice(const QString &deviceName)
{
	// TODO: Use a QSet instead of QStringList?
	if (cdromDeviceNames.contains(deviceName)) {
		// Device is already added.
		// TODO: Refresh the device.
		return;
	}

	// Add the CD-ROM drive.
	Q_Q(FindCdromDrives);
	cdromDeviceNames.append(deviceName);
	emit q->driveAdded(deviceName);
}

/**
 * Remove a CD-ROM device.
 * @param deviceName CD-ROM device name.
 */
void FindCdromDrivesPrivate::removeCdromDevice(const QString &deviceName)
{
	// TODO: Use a QSet instead of QStringList?
	if (!cdromDeviceNames.contains(deviceName)) {
		// Device is not present.
		return;
	}

	// Remove the CD-ROM drive.
	cdromDeviceNames.removeOne(deviceName);

	// Delete the LibGensCD::CdDrive instance.
	LibGensCD::CdDrive *cdDrive = mapCdromDevices.value(deviceName);
	if (cdDrive) {
		delete cdDrive;
		mapCdromDevices.remove(deviceName);
	}

	// Drive is removed.
	Q_Q(FindCdromDrives);
	emit q->driveRemoved(deviceName);
}

/**
 * Get the icon for a given drive type.
 * @param driveType Drive type.
 * @return Drive type icon, or invalid icon if the drive type is either DRIVE_TYPE_NONE or invalid.
 */
QIcon FindCdromDrivesPrivate::GetDriveTypeIcon(CD_DriveType_t driveType)
{
	// TODO: Fallback icons.
	// Possibly use Windows or Mac system icons.
	// TODO: Figure out what QStyle::SP_DriveCDIcon and QStyle::SP_DriveDVDIcon are.

	// TODO: Add icons for different types of drives.
	Q_UNUSED(driveType)
	return GensQApplication::IconFromTheme(QLatin1String("drive-optical"));
}

/**
 * Get the icon for a given disc type.
 * NOTE: The disc type MUST be a single bit! (Combinations are treated as invalid.)
 * @param discType Disc type.
 * @return Disc type icon, or invalid icon if the disc type is DISC_TYPE_NONE or invalid.
 */
QIcon FindCdromDrivesPrivate::GetDiscTypeIcon(CD_DiscType_t discType)
{
	// TODO: Fallback icons.
	// Possibly use Windows or Mac system icons.
	// TODO: Figure out what QStyle::SP_DriveCDIcon and QStyle::SP_DriveDVDIcon are.

	// TODO: Add more unique icons.
	QString iconFdo;
	switch (discType) {
		case DISC_TYPE_NONE:
		default:
			return QIcon();

		case DISC_TYPE_CDROM:
			iconFdo = QLatin1String("media-optical");
			break;

		case DISC_TYPE_CD_R:
		case DISC_TYPE_CD_RW:
			iconFdo = QLatin1String("media-optical-recordable");
			break;

		case DISC_TYPE_DVDROM:
			iconFdo = QLatin1String("media-optical-dvd");
			break;

		case DISC_TYPE_DVD_R:
		case DISC_TYPE_DVD_RAM:
		case DISC_TYPE_DVD_RW:
		case DISC_TYPE_DVD_R_DL:
		case DISC_TYPE_DVD_RW_DL:
		case DISC_TYPE_DVD_PLUS_RW:
		case DISC_TYPE_DVD_PLUS_R:
		case DISC_TYPE_DVD_PLUS_RW_DL:
		case DISC_TYPE_DVD_PLUS_R_DL:
			iconFdo = QLatin1String("media-optical-recordable");
			break;

		case DISC_TYPE_DDCDROM:
			iconFdo = QLatin1String("media-optical");
			break;

		case DISC_TYPE_DDCD_R:
		case DISC_TYPE_DDCD_RW:
			iconFdo = QLatin1String("media-optical-recordable");
			break;

		case DISC_TYPE_BDROM:
		case DISC_TYPE_BD_R:
		case DISC_TYPE_BD_RE:
			iconFdo = QLatin1String("media-optical-blu-ray");
			break;

		case DISC_TYPE_HDDVD:
			iconFdo = QLatin1String("media-optical-dvd");
			break;

		case DISC_TYPE_HDDVD_R:
		case DISC_TYPE_HDDVD_RAM:
		case DISC_TYPE_HDDVD_RW:
		case DISC_TYPE_HDDVD_R_DL:
		case DISC_TYPE_HDDVD_RW_DL:
			iconFdo = QLatin1String("media-optical-recordable");
			break;

		case DISC_TYPE_MO:
			iconFdo = QLatin1String("media-optical-recordable");
			break;
	}

	return GensQApplication::IconFromTheme(iconFdo);
}

/** FindCdromDrives **/

FindCdromDrives::FindCdromDrives(QObject *parent)
	: super(parent)
	, d_ptr(new FindCdromDrivesPrivate(this))
{
	// Rescan disc drives.
	rescan();
}

FindCdromDrives::~FindCdromDrives()
{
	delete d_ptr;
}

/**
 * Check if CD-ROM drives are supported on this platform.
 * @return True if supported; false if not.
 */
bool FindCdromDrives::isSupported(void) const
{
	Q_D(const FindCdromDrives);
	return (d->findCdromBase && d->findCdromBase->isUsable());
}

/**
 * Rescan all disc drives.
 * This clears the QStringList and enumerates all disc drives.
 */
void FindCdromDrives::rescan(void)
{
	Q_D(FindCdromDrives);
	d->clearCdromDevices();

	// Search for drives.
	if (d->findCdromBase) {
		QStringList cdromDeviceNames = d->findCdromBase->scanDeviceNames();
		foreach (QString deviceName, cdromDeviceNames) {
			d->addCdromDevice(deviceName);
		}
	}
}

/**
 * Get a list of all available CD-ROM device names.
 * @return QStringList containing CD-ROM device names.
 */
QStringList FindCdromDrives::getDriveNames(void) const
{
	Q_D(const FindCdromDrives);
	return d->cdromDeviceNames;
}

/**
 * Get a libgenscd CdDrive instance for a given CD-ROM device name.
 * @param deviceName CD-ROM device name.
 * @return CdDrive instance, or nullptr if unable to open the drive.
 */
LibGensCD::CdDrive *FindCdromDrives::getCdDrive(const QString &deviceName)
{
	// TODO: Use a QSet instead of QStringList?
	Q_D(FindCdromDrives);
	if (!d->cdromDeviceNames.contains(deviceName)) {
		// Device is not present.
		return nullptr;
	}

	// Check if we already have an instance of LibGensCD::CdDrive for this device.
	LibGensCD::CdDrive *cdDrive = d->mapCdromDevices.value(deviceName);
	if (cdDrive)
		return cdDrive;

	// Create an instance of LibGensCD::CdDrive.
	cdDrive = new LibGensCD::CdDrive(deviceName.toUtf8().constData());
	if (!cdDrive->isOpen()) {
		// Could not open the CD-ROM drive.
		// TODO: Show an error.
		fprintf(stderr, "FindCdromDrivesPrivate::addCdromDevice(): ERROR opening device %s\n",
			deviceName.toUtf8().constData());
		delete cdDrive;
		return nullptr;
	}

	d->mapCdromDevices.insert(deviceName, cdDrive);
	return cdDrive;
}

/**
 * Get the icon for the specified CD-ROM drive.
 * Returns the disc icon if a disc is present,
 * or the drive icon if no disc is present.
 * @param deviceName CD_ROM device name.
 * @return Icon for either the disc or the drive.
 */
QIcon FindCdromDrives::getDriveIcon(const QString &deviceName)
{
	LibGensCD::CdDrive *cdDrive = getCdDrive(deviceName);
	if (cdDrive)
		return getDriveIcon(cdDrive);

	// Error obtaining CdDrive for this device.
	return QIcon();
}

/**
 * Get the icon for the specified CD-ROM drive.
 * Returns the disc icon if a disc is present,
 * or the drive icon if no disc is present.
 * @param cdDrive LibGensCD::CdDrive.
 * @return Icon for either the disc or the drive.
 */
QIcon FindCdromDrives::getDriveIcon(LibGensCD::CdDrive *cdDrive)
{
	if (!cdDrive)
		return QIcon();

	// Check if the backend has an OS-specific disc/drive icon function.
	Q_D(FindCdromDrives);
	if (d->findCdromBase->isDriveIconSupported()) {
		// OS-specific disc/drive icon function is present.
		QIcon icon = d->findCdromBase->getDriveIcon(QString::fromUtf8(cdDrive->filename().c_str()));
		if (!icon.isNull())
			return icon;
	}

	if (cdDrive->isDiscPresent()) {
		// Disc is present.
		// Get the disc type icon.
		return d->GetDiscTypeIcon(cdDrive->getDiscType());
	} else {
		// Disc is not present.
		// Get the drive type icon.
		return d->GetDriveTypeIcon(cdDrive->getDriveType());
	}

	// Should not get here...
	return QIcon();
}

}
