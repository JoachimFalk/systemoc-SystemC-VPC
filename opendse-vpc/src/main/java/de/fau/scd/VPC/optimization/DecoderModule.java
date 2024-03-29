// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
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

package de.fau.scd.VPC.optimization;

import org.opt4j.core.config.Icons;
import org.opt4j.core.config.annotations.Category;
import org.opt4j.core.config.annotations.Icon;

import com.google.inject.multibindings.Multibinder;

import net.sf.opendse.optimization.DesignSpaceExplorationModule;

@Category
@Icon(Icons.PUZZLE_BLUE)
public abstract class DecoderModule extends DesignSpaceExplorationModule {

        protected void bindCreatorDecoder(Class<? extends ImplementationCreatorDecoder> clazz) {
                Multibinder<ImplementationCreatorDecoder> multibinder = Multibinder.newSetBinder(binder(),
                        ImplementationCreatorDecoder.class);
                multibinder.addBinding().to(clazz).asEagerSingleton();
        }
}
