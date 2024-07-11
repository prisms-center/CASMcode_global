import os

# -- package specific configuration --
project = "libcasm-global"
version = "2.0"  # The short X.Y version.
release = "2.0.4"  # The full version, including alpha/beta/rc tags.
project_desc = "CASM global constants and tools"
logo_text = "libcasm-global"
github_url = "https://github.com/prisms-center/CASMcode_global/"
pypi_url = "https://pypi.org/project/libcasm-global/"
intersphinx_libcasm_packages = []


# -- CASM common configuration ---

# -*- coding: utf-8 -*-
#
# CASM documentation build configuration file, created by
# sphinx-quickstart on Sat Sep 16 00:49:21 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.


# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

autoclass_content = "both"
# autodoc_class_signature = "separated"
autosummary_generate = True
autosummary_imported_members = True
numpydoc_show_class_members = False
# autodoc_typehints = 'both'
autodoc_typehints_format = "short"
python_use_unqualified_type_names = True
autodoc_inherit_docstrings = False
add_module_names = True

intersphinx_mapping = {}

# if LIBCASM_LOCAL_PYDOCS env variable is set, create local docs
pydocs_path = os.environ.get("LIBCASM_LOCAL_PYDOCS", None)
for package, vers in intersphinx_libcasm_packages:
    if pydocs_path is None:
        url = (
            f"https://prisms-center.github.io/CASMcode_pydocs/libcasm/{package}/{vers}/"
        )
        inventory = None
    else:
        url = os.path.join(pydocs_path, f"{package}/{vers}/html")
        inventory = os.path.join(pydocs_path, f"{package}/{vers}/objects.inv")
    intersphinx_mapping[package] = (url, inventory)

print(intersphinx_mapping)


# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.coverage",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
    "sphinx.ext.napoleon",
    "sphinxarg.ext",
    "sphinxcontrib.bibtex",
    "sphinx.ext.intersphinx",
    "numpydoc",
]

bibtex_bibfiles = ["refs.bib"]

# Napoleon settings
napoleon_google_docstring = False
napoleon_numpy_docstring = True
python_maximum_signature_line_length = 20

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = ".rst"

# The master toctree document.
master_doc = "index"

# General information about the project.
copyright = "2023, CASM Developers"
author = "CASM Developers"

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#


# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
# pygments_style = "sphinx"

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False

# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "pydata_sphinx_theme"

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
html_logo = "_static/small_logo.svg"
html_theme_options = {
    "logo": {
        "text": logo_text,
        "image_light": "_static/small_logo.svg",
        "image_dark": "_static/small_logo_dark.svg",
    },
    "pygment_light_style": "xcode",
    "pygment_dark_style": "lightbulb",
    "icon_links": [
        {
            # Label for this link
            "name": "GitHub",
            "url": github_url,  # required
            "icon": "fa-brands fa-github",
            "type": "fontawesome",
        },
        {
            # Label for this link
            "name": "PyPI",
            "url": pypi_url,  # required
            "icon": "fa-brands fa-python",
            "type": "fontawesome",
        },
    ],
    "favicons": [
        {
            "rel": "icon",
            "sizes": "32x32",
            "href": "favicon-32x32.png",
        },
        {
            "rel": "icon",
            "sizes": "16x16",
            "href": "favicon-16x16.png",
        },
        {"rel": "apple-touch-icon", "sizes": "180x180", "href": "apple-touch-icon.png"},
    ],
}
# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]
# html_favicon = "_static/small_logo.svg"
# favicons = ["small_logo.svg"]
html_css_files = [
    "css/custom.css",
]

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
# html_sidebars = {
#     '**': [
#         'about.html',
#         'navigation.html',
#         'relations.html',  # needs 'show_related': True theme option to display
#         'searchbox.html',
#         'donate.html',
#     ]
# }

# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = "CASMdoc"

# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',
    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',
    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',
    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (
        master_doc,
        f"{project}.tex",
        f"{project} Documentation",
        "CASM Developers",
        "manual",
    ),
]

# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [(master_doc, f"{project}", f"{project} Documentation", [author], 1)]

# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (
        master_doc,
        f"{project}",
        f"{project} Documentation",
        author,
        f"{project}",
        project_desc,
        "Miscellaneous",
    ),
]
