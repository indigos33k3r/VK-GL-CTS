/*-------------------------------------------------------------------------
 * Vulkan CTS Framework
 * --------------------
 *
 * Copyright (c) 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief RenderDoc utility
 *//*--------------------------------------------------------------------*/

#include "vkRenderDocUtil.hpp"

#include "deDynamicLibrary.hpp"
#include "deUniquePtr.hpp"
#include "tcuDefs.hpp"

#include "renderdoc_app.h"
#include <stdexcept>

#if (DE_OS == DE_OS_WIN32)
#	define RENDERDOC_LIBRARY_NAME "renderdoc.dll"
#else
#	define RENDERDOC_LIBRARY_NAME "librenderdoc.so"
#endif

namespace vk
{

struct RenderDocPrivate
{
										RenderDocPrivate	(void)	: m_api(DE_NULL), m_valid(false) {}

	de::MovePtr<de::DynamicLibrary>		m_library;
	::RENDERDOC_API_1_1_2*				m_api;
	bool								m_valid;
};

RenderDocUtil::RenderDocUtil (void)
	: m_priv	(new RenderDocPrivate)
{
	try
	{
		m_priv->m_library	 = de::MovePtr<de::DynamicLibrary>(new de::DynamicLibrary(RENDERDOC_LIBRARY_NAME));
	}
	catch (const std::runtime_error& e)
	{
		tcu::print("Library %s not loaded: %s, RenderDoc API not available", e.what(), RENDERDOC_LIBRARY_NAME);
	}

	if (m_priv->m_library)
	{
		::pRENDERDOC_GetAPI pGetApi = (::pRENDERDOC_GetAPI)m_priv->m_library->getFunction("RENDERDOC_GetAPI");
		const int			ret		= pGetApi(eRENDERDOC_API_Version_1_1_2, (void **)&m_priv->m_api);

		if (ret == 1)
		{
			m_priv->m_api->TriggerCapture();

			m_priv->m_valid = true;
		}
		else
		{
			tcu::print("RENDERDOC_GetAPI returned %d status, RenderDoc API not available", ret);
		}
	}
}

RenderDocUtil::~RenderDocUtil (void)
{
	if (m_priv)
	{
		delete m_priv;
	}
}

bool RenderDocUtil::isValid (void)
{
	return m_priv != DE_NULL && m_priv->m_valid;
}

void RenderDocUtil::startFrame (vk::VkInstance instance)
{
	if (!isValid()) return;

	m_priv->m_api->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance), DE_NULL);
}

void RenderDocUtil::endFrame (vk::VkInstance instance)
{
	if (!isValid()) return;

	m_priv->m_api->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance), DE_NULL);
}

} // vk