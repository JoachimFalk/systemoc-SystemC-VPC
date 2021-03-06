// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

package de.fau.scd.VPC.config.visualization;

import java.awt.Component;
//import java.awt.Dimension;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.lang.reflect.InvocationTargetException;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import org.opt4j.core.config.Property;
import org.opt4j.core.config.PropertyModule;
import org.opt4j.core.config.visualization.FileChooser;
import org.opt4j.core.config.visualization.Format;

import de.fau.scd.VPC.config.annotations.Text;
import de.fau.scd.VPC.config.properties.AttributeAnnotations;
import de.fau.scd.VPC.config.properties.AttributeLogs;
import de.fau.scd.VPC.config.properties.Environment;
import de.fau.scd.VPC.config.properties.Objectives;

@SuppressWarnings("serial")
public class PropertyPanel extends org.opt4j.core.config.visualization.PropertyPanel {

    public PropertyPanel(PropertyModule module, FileChooser fileChooser, Format format) {
        super(module, fileChooser, format);
    }

    protected Component createComponent(final Property property) {
        Class<?> type  = property.getType();
        Object   value = property.getValue();

        Text textAnotation = property.getAnnotation(Text.class);

        if (type.isAssignableFrom(Environment.class)) {
            return new EnvironmentPanel(property);
        } else if (type.isAssignableFrom(Objectives.class)) {
            return new ObjectivesPanel(property);
        } else if (type.isAssignableFrom(AttributeAnnotations.class)) {
            return new AttributeAnnotationsPanel(property);
        } else if (type.isAssignableFrom(AttributeLogs.class)) {
            return new AttributeLogsPanel(property);
        } else if (textAnotation != null && type.isAssignableFrom(String.class)) {
            final JTextArea   field = new JTextArea(textAnotation.rows(), 0);
            final JScrollPane scroll = new JScrollPane(field);

            field.setLineWrap(true);
            field.setWrapStyleWord(true);

            if (value == null) {
                field.setText("");
            } else {
                field.setText(value.toString());
            }

            field.addFocusListener(new FocusAdapter() {
                @Override
                public void focusLost(FocusEvent ev) {
                    String value = format(property, field.getText());
                    try {
                        property.setValue(value);
                    } catch (InvocationTargetException ex) {
                        Throwable e = ex;
                        while (e != null && e instanceof InvocationTargetException) {
                            e = ((InvocationTargetException) e).getCause();
                        }
                        if (e != null && e.getMessage() != null)
                            System.err.println(e.getMessage());
                    } finally {
                        field.setText(property.getValue().toString());
                        update();
                    }
                }
            });

            field.addKeyListener(new KeyAdapter() {

                @Override
                public void keyReleased(KeyEvent ev) {
                    String value = format(property, field.getText());

                    try {
                        property.setValue(value);
                    } catch (InvocationTargetException ex) {
                        Throwable e = ex;
                        while (e != null && e instanceof InvocationTargetException) {
                            e = ((InvocationTargetException) e).getCause();
                        }
                        if (e != null && e.getMessage() != null)
                            System.err.println(e.getMessage());
                    }
                }
            });

            scroll.setMinimumSize(scroll.getPreferredSize());

//          System.out.println(scroll.getMinimumSize());
//          System.out.println(scroll.getPreferredSize());
//          System.out.println(scroll.getSize());

            return scroll;
        } else {
            return super.createComponent(property);
        }
    }

}
