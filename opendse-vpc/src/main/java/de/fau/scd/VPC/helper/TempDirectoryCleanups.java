// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Martin Letras <martin.letras@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.helper;

import java.io.File;
import java.util.HashSet;
import java.util.Iterator;

/**
 *
 * @author
 *
 */
@SuppressWarnings("serial")
public class TempDirectoryCleanups extends HashSet<ITempDirectoryCleanup> {

    private final File tmpDir;

    public TempDirectoryCleanups(File f) {
        this.tmpDir = f;
    }

    /*
     * (non-Javadoc)
     *
     * @see java.util.HashSet#add(java.lang.Object)
     */
    @Override
    public boolean add(ITempDirectoryCleanup cleanUpJob) {
        return (super.add(cleanUpJob));
    }

    /**
     * Uses all customized cleanups to cleanup the temporary directory.
     *
     */
    public boolean cleanUpTmpDir() {
        boolean success = false;
        for (ITempDirectoryCleanup c : this) {
            success = c.cleanup(tmpDir);
            if (!success) {
                return success;
            }
        }
        return success;
    }

    /*
     * (non-Javadoc)
     *
     * @see java.util.HashSet#iterator()
     */
    @Override
    public Iterator<ITempDirectoryCleanup> iterator() {

        Iterator<ITempDirectoryCleanup> iterator = new Iterator<ITempDirectoryCleanup>() {
            final Iterator<ITempDirectoryCleanup> i = TempDirectoryCleanups.super.iterator();

            @Override
            public boolean hasNext() {
                return i.hasNext();
            }

            @Override
            public ITempDirectoryCleanup next() {
                return i.next();
            }

            @Override
            public void remove() {
                throw new RuntimeException("Remove with Iterator of ITempDirectoryCleanup not supported");
            }
        };

        return iterator;
    }

    /*
     * (non-Javadoc)
     *
     * @see java.util.HashSet#remove(java.lang.Object)
     */
    @Override
    public boolean remove(Object obj) {
        if (obj instanceof ITempDirectoryCleanup && super.remove(obj)) {
            return true;
        }
        return false;
    }
}
